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

    member this.AllOutputLayers = layers :> IEnumerable<OutputLayer>
    member this.AllOutputConstructions = constructions :> IEnumerable<OutputConstruction>

    member this.ConstructionNameForLayers(constructions, thicknesses) =
        match List.ofSeq constructions with
        | Empty -> 
            let layer = retrieveLayer (OutputLayerInfraredTransparent())
            (retrieveConstruction ([layer]) None).Name
        | MappedWindow(name, libraryLayers) -> 
            let layerCopies = libraryLayers |> List.map(retrieveCopy None)
            (retrieveConstruction layerCopies (Some(name))).Name
        | SimpleOnly(infos) -> 
            let ts = thicknesses |> Seq.map (fun t -> Some(t)) |> List.ofSeq
            let outputLayers = List.map2 retrieveCopy ts infos
            (retrieveConstruction outputLayers None).Name
        | SingleComposite(name, layers) ->
            // If the thicknesses don't match: too bad.
            let outputLayers = 
                layers |> 
                List.map (fun (entry, thickness) -> 
                    retrieveCopy (Some(thickness)) entry)
            (retrieveConstruction outputLayers name).Name
        | UnmappedWindow -> "WINDOW WITH UNMAPPED CONSTRUCTION"
        | MixedSingleAndComposite -> 
            "UNSUPPORTED MAPPING (MIXED SINGLE MATERIALS AND COMPOSITES)"
        | Unknown -> "UNSUPPORTED MAPPING (UNKNOWN)"

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