namespace ConstructionManagement

open System
open System.Collections.Generic

open MaterialLibrary
open ConstructionManagement.ModelConstructions
open OutputPatterns

type OutputManager (warnDelegate : Action<string>) =
    let mutable layers = Set.empty
    let mutable constructions = Set.empty

    let warn =
        let warnings = ref Set.empty
        fun w ->
            if not (Set.contains w !warnings) then 
                warnDelegate.Invoke(w)
                warnings := Set.add w !warnings

    let retrieveConstruction materials humanReadableName =
        let newC = OutputConstruction(materials, humanReadableName, warn)
        constructions <- constructions.Add(newC)
        newC

    let retrieveLayer (layer:OutputLayer) =
        layers <- layers.Add(layer)
        layer

    let retrieveOpaqueLayer (libraryEntry:LibraryEntryOpaque) thickness =
        let n = sprintf "%s (%.3f)" (libraryEntry.Name.ToString()) thickness
        let name =
            if n.Length <= 100 
            then n 
            else sprintf "Material id %i" (n.GetHashCode())
        retrieveLayer (OutputLayerOpaque(name, libraryEntry, thickness))

    let retrieveOpaqueSurface (libraryEntry:LibraryEntryOpaque) =
        let n = sprintf "%s (surface only)" (libraryEntry.Name.ToString())
        let name =
            if n.Length <= 100
            then n
            else sprintf "Material id %i" (n.GetHashCode())
        retrieveLayer (OutputLayerOpaque(name, libraryEntry, 0.001))

    let retrieveCopy (newThickness: double option) (libraryEntry:LibraryEntry) =
        match libraryEntry, newThickness with
        | LibraryEntry.AirGap(props), _ -> retrieveLayer (OutputLayerAirGap(float props.ThermalResistance))
        | LibraryEntry.Composite(_), _ -> raise (ArgumentException())
        | LibraryEntry.Gas(props), _ -> retrieveLayer (OutputLayerGas(props.Name.ToString(), props, float props.Thickness))
        | LibraryEntry.Glazing(props), _ -> retrieveLayer (OutputLayerGlazing(props.Name.ToString(), props, float props.Thickness))
        | LibraryEntry.NoMass(props), _ -> retrieveLayer (OutputLayerNoMass(props.Name.ToString(), props))
        | LibraryEntry.Opaque(props), Some(t) -> retrieveOpaqueLayer props t
        | LibraryEntry.Opaque(props), None -> retrieveOpaqueLayer props (float props.Thickness)

    let cloneLibraryComposite withName layers =
        let layerCopies = layers |> List.map (retrieveCopy None)
        retrieveConstruction layerCopies withName

    let rec partial layers depth soFar =
        match layers, depth with
        | (nextE, nextT) :: rest, d when d >= nextT ->
            partial rest (d - nextT) (soFar @ [nextE, nextT])
        | (nextE, nextT) :: rest, d when d < nextT ->
            soFar @ [nextE, d]
        | _ -> soFar
        // The last case can be reached either in the case that the program is
        // in a state that it shouldn't be in or the case that fp imprecision
        // has mucked something up. I don't think it's worth trying to
        // eliminate the fp issue.

    let retrievePartialComposite origName layers depth reversed =
        let checkLayers = if reversed then List.rev layers else layers
        let outputLayers =
            partial checkLayers depth [] |>
            List.map (fun (e, t) -> retrieveCopy (Some(t)) e)
        retrieveConstruction outputLayers origName

    member this.AllOutputLayers = layers :> IEnumerable<OutputLayer>
    member this.AllOutputConstructions = constructions :> IEnumerable<OutputConstruction>

    member this.ConstructionNameForLayers(surfaceNormal,
                                          constructions, 
                                          normals, 
                                          thicknesses) =
        let cnorms = normals |> Seq.map (function
            | 0.0, 0.0, 0.0 -> None
            | n -> Some(n))
        match List.ofSeq constructions with
        | Empty -> 
            let layer = retrieveLayer (OutputLayerInfraredTransparent())
            (retrieveConstruction ([layer]) None).Name
        | MappedWindow(name, libraryLayers) -> 
            (cloneLibraryComposite (Some(name)) libraryLayers).Name
        | SimpleOnly(infos) -> 
            let ts = thicknesses |> Seq.map (fun t -> Some(t)) |> List.ofSeq
            let outputLayers = List.map2 retrieveCopy ts infos
            (retrieveConstruction outputLayers None).Name
        | SingleComposite(name, layers) ->
            let compositeNorm = Seq.head cnorms
            if compositeNorm.IsNone then
                warn "There was an attempt to generate a composite from an \
                      un-oriented material layer set. This can happen if the \
                      original layer set was improperly defined as a material \
                      list. The construction has been named 'UNORIENTED \
                      COMPOSITE' in the IDF. You will have to create this \
                      construction manually."
                "UNORIENTED COMPOSITE" else
            let dotproduct (ax, ay, az) (bx, by, bz) =
                ax * bx + ay * by + az * bz
            let magSq (d: float * float * float) = dotproduct d d
            let magnitude d = sqrt (magSq d)
            let areParallel a b = 
                let lhs = dotproduct a b 
                let rhs = (magnitude a) * (magnitude b)
                abs(lhs - rhs) < 0.001  || abs(lhs + rhs) < 0.001 // magic eps
            if not (areParallel surfaceNormal compositeNorm.Value) then
                warn
                    "A surface's assigned material layers were not parallel \
                     to its normal. The construction has been named \
                     'UNALIGNED COMPOSITE' in the IDF. You will have to \
                     create this construction manually."
                "UNALIGNED COMPOSITE" else
            let areAntiparallel a b =
                let ax, ay, az = a
                let bx, by, bz = b
                let sum = ax + bx, ay + by, az + bz
                magSq sum < magSq a + magSq b
            let reversed = areAntiparallel surfaceNormal compositeNorm.Value
            let reqThickness = Seq.head thicknesses
            let c = retrievePartialComposite name layers reqThickness reversed
            c.Name
        | _ ->
            warn "An element configuration was too complicated to have its \
                  construction automatically generated. It has been assigned \
                  the construction name 'UNMAPPED'. You will have to create \
                  this construction manually."
            "UNMAPPED"

    member this.ConstructionNameForSurface(c:ModelConstruction) = 
        match c with
        | ModelConstruction.SingleOpaque(src) ->
            match src.MappingTarget with
            | noMapping when noMapping = Unchecked.defaultof<LibraryEntry> -> 
                "MISSING MAPPING FOR THIRD-LEVEL SURFACE"
            | LibraryEntry.Opaque(entry) -> 
                let surfaceLayer = retrieveOpaqueSurface entry
                (retrieveConstruction [surfaceLayer] None).Name
            | LibraryEntry.Composite(_) -> 
                "INVALID MAPPING (SURFACE FOR COMPOSITE LIBRARY ENTRY)"
            | _ -> "INVALID MAPPING (OPAQUE TO NON-OPAQUE)"
        | ModelConstruction.Window(_) -> 
            "COULDN'T BUILD CONSTRUCTION (THIRD-LEVEL WINDOW SURFACE)"
        | ModelConstruction.LayerSet(_, layers) ->
            let firstLayer = fst layers.[0]
            // Assumption: symmetrical.
            match firstLayer.MappingTarget with
            | noMapping when noMapping = Unchecked.defaultof<LibraryEntry> -> 
                "MISSING MAPPING FOR THIRD-LEVEL SURFACE"
            | LibraryEntry.Opaque(entry) -> 
                let surfaceLayer = retrieveOpaqueSurface entry
                (retrieveConstruction [surfaceLayer] None).Name
            | entry -> 
                (retrieveConstruction [retrieveCopy None entry] None).Name
            | _ -> "BAD MAPPING FOR THIRD-LEVEL SURFACE"