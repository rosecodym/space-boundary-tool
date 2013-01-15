namespace ConstructionManagement

open System
open System.Collections.Generic

open MaterialLibrary
open ConstructionManagement.ModelConstructions
open OutputPatterns

type OutputManager (warnDelegate : Action<string>) =
    let mutable layers = Set.empty
    let mutable constructions = Set.empty
    let mutable variantTable: Map<string, int> = Map.empty

    let warn =
        let warnings = ref Set.empty
        fun w ->
            if not (Set.contains w !warnings) then 
                warnDelegate.Invoke(w)
                warnings := Set.add w !warnings

    let retrieveConstruction materials humanReadableName =
        let getVariant = fun ident -> Map.tryFind ident variantTable
        let newC = OutputConstruction(materials, humanReadableName, getVariant)
        Seq.iter warn newC.Warnings
        constructions <- Set.add newC constructions
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

    let getThickestLayer layers = Seq.maxBy snd layers |> fst

    let dotproduct (ax, ay, az) (bx, by, bz) =
        ax * bx + ay * by + az * bz
    let magSq (d: float * float * float) = dotproduct d d
    let magnitude d = sqrt (magSq d)
    let areParallel a b = 
        let lhs = dotproduct a b 
        let rhs = (magnitude a) * (magnitude b)
        abs(lhs - rhs) < 0.001  || abs(lhs + rhs) < 0.001 // magic eps
    let areAntiparallel a b =
        let ax, ay, az = a
        let bx, by, bz = b
        let sum = ax + bx, ay + by, az + bz
        magSq sum < magSq a + magSq b

    member this.AllOutputLayers = layers :> IEnumerable<OutputLayer>
    member this.AllOutputConstructions =
        constructions :> IEnumerable<OutputConstruction>

    member this.ConstructionForLayers(surfaceNormal,
                                      constructions, 
                                      normals, 
                                      thicknesses) : OutputConstructionBase =
        let emitProblemConstruction (c: ProblemConstruction) =
            warn c.Message
            c :> OutputConstructionBase
        let cnorms = normals |> Seq.map (function
            | 0.0, 0.0, 0.0 -> None
            | n -> Some(n))
        match List.ofSeq constructions with
        | Empty -> 
            let layer = retrieveLayer (OutputLayerInfraredTransparent())
            upcast retrieveConstruction ([layer]) None
        | MappedWindow(name, libraryLayers) -> 
            upcast cloneLibraryComposite (Some(name)) libraryLayers 
        | SimpleOnly(infos) -> 
            let ts = thicknesses |> Seq.map (fun t -> Some(t)) |> List.ofSeq
            let outputLayers = List.map2 retrieveCopy ts infos
            upcast retrieveConstruction outputLayers None
        | SingleComposite(name, layers) ->
            let reqThickness = Seq.head thicknesses
            let compositeNorm = Seq.head cnorms
            if compositeNorm.IsNone then
                emitProblemConstruction (UnorientedComposite()) else
            if not (areParallel surfaceNormal compositeNorm.Value) then
                let thickest = getThickestLayer layers
                let layer = retrieveCopy (Some(reqThickness)) thickest
                upcast retrieveConstruction [layer] None else
            let reversed = areAntiparallel surfaceNormal compositeNorm.Value
            let c = retrievePartialComposite name layers reqThickness reversed
            upcast c
        | _ ->
            emitProblemConstruction (UnknownProblemComposite())

    member this.ConstructionForSurface (surfaceNormal,
                                        constructionNormal,
                                        c) : OutputConstructionBase = 
        let emitProblemConstruction (c: ProblemConstruction) =
            warn c.Message
            c :> OutputConstructionBase
        match c with
        | ModelConstruction.SingleOpaque(src) ->
            match src.MappingTarget with
            | noMapping when noMapping = Unchecked.defaultof<LibraryEntry> -> 
                emitProblemConstruction (BadMappingConstruction())
            | LibraryEntry.Opaque(entry) -> 
                let surfaceLayer = retrieveOpaqueSurface entry
                upcast retrieveConstruction [surfaceLayer] None
            | LibraryEntry.Composite(_) -> 
                emitProblemConstruction (LibraryComposite())
            | _ -> emitProblemConstruction (BadMappingConstruction())
        | ModelConstruction.Window(_) -> 
            emitProblemConstruction (AdiabaticWindowConstruction())
        | ModelConstruction.LayerSet(_, layers) ->
            let relevantLayer =
                if not (areParallel surfaceNormal constructionNormal) then
                    getThickestLayer layers
                else if areAntiparallel surfaceNormal constructionNormal then
                    layers |> List.rev |> Seq.head |> fst
                else
                    layers.Head |> fst
            match relevantLayer.MappingTarget with
            | noMap when noMap = Unchecked.defaultof<LibraryEntry> ->
                emitProblemConstruction (BadMappingConstruction())
            | LibraryEntry.Opaque(entry) ->
                let surfaceLayer = retrieveOpaqueSurface entry
                upcast retrieveConstruction [surfaceLayer] None
            | e ->
                // I don't know what this case is for.
                let res = retrieveConstruction [retrieveCopy None e] None
                upcast res
            | _ -> emitProblemConstruction (BadMappingConstruction())

    member this.IdentifyConstructionVariants () =
        variantTable <-
            constructions |>
            Seq.groupBy (fun c -> c.InvariantName) |>
            Seq.map snd |>
            Seq.filter (Seq.length >> (<>) 1) |>
            Seq.collect (Seq.mapi (fun v c -> c.Identifier, v)) |>
            Map.ofSeq