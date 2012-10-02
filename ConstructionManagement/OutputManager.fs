namespace ConstructionManagement

open System
open System.Collections.Generic

open MaterialLibrary
open ConstructionManagement.ModelConstructions
open OutputPatterns

type OutputManager () =
    let mutable layers = Set.empty
    let mutable constructions = Set.empty

    let retrieveConstruction materials humanReadableName =
        let newC = OutputConstruction(materials, humanReadableName)
        constructions <- constructions.Add(newC)
        newC

    let retrieveLayer (layer:OutputLayer) =
        layers <- layers.Add(layer)
        layer

    let retrieveOpaqueLayer (libraryEntry:LibraryEntryOpaque) thickness =
        retrieveLayer (OutputLayerOpaque(sprintf "%s (%.3f)" (libraryEntry.Name.ToString()) thickness, libraryEntry, thickness))

    let retrieveOpaqueSurface (libraryEntry:LibraryEntryOpaque) =
        retrieveLayer (OutputLayerOpaque(sprintf "%s (surface only)" (libraryEntry.Name.ToString()), libraryEntry, 0.001))

    let retrieveOpaqueOrAirGapLayer (libraryEntry:LibraryEntry) thickness = // this is gross but i can't think of an easier, better way
        match libraryEntry with
        | LibraryEntry.Opaque(props) -> retrieveOpaqueLayer props thickness
        | LibraryEntry.AirGap(props) -> retrieveLayer (OutputLayerAirGap(float props.ThermalResistance))
        | _ -> raise (ArgumentException())

    let retrieveExactCopy (libraryEntry:LibraryEntry) =
        match libraryEntry with
        | LibraryEntry.AirGap(props) -> retrieveLayer (OutputLayerAirGap(float props.ThermalResistance))
        | LibraryEntry.Composite(_) -> raise (ArgumentException())
        | LibraryEntry.Gas(props) -> retrieveLayer (OutputLayerGas(props.Name.ToString(), props, float props.Thickness))
        | LibraryEntry.Glazing(props) -> retrieveLayer (OutputLayerGlazing(props.Name.ToString(), props, float props.Thickness))
        | LibraryEntry.Opaque(props) -> retrieveOpaqueLayer props (float props.Thickness)

    member this.AllOutputLayers = layers :> IEnumerable<OutputLayer>
    member this.AllOutputConstructions = constructions :> IEnumerable<OutputConstruction>

    member this.ConstructionNameForLayers(constructions:IList<ModelConstruction>, thicknesses:IList<double>) =
        match Array.ofSeq constructions with
        | Empty -> (retrieveConstruction (Array.create 1 (retrieveLayer (OutputLayerInfraredTransparent()))) None).Name
        | MappedWindow(name, libraryLayers) -> (retrieveConstruction (libraryLayers |> Array.map retrieveExactCopy) (Some(name))).Name
        | SimpleOnly(infos) -> 
            let outputLayers = Array.map2 retrieveOpaqueOrAirGapLayer infos (Array.ofSeq thicknesses)
            (retrieveConstruction outputLayers None).Name
        | SingleComposite(name, layers) -> // if the thicknesses don't match, then too bad
            let outputLayers = layers |> Array.map (fun (entry, thickness) -> retrieveOpaqueOrAirGapLayer entry thickness)
            (retrieveConstruction outputLayers name).Name
        | UnmappedWindow -> "WINDOW WITH UNMAPPED CONSTRUCTION"
        | MixedSingleAndComposite -> "UNSUPPORTED MAPPING (MIXED SINGLE MATERIALS AND COMPOSITES)"
        | Unknown -> "UNSUPPORTED MAPPING (UNKNOWN)"

    member this.ConstructionNameForSurface(c:ModelConstruction) = 
        match c with
        | ModelConstruction.SingleOpaque(src) ->
            match src.MappingTarget with
            | noMapping when noMapping = Unchecked.defaultof<LibraryEntry> -> "MISSING MAPPING FOR THIRD-LEVEL SURFACE"
            | LibraryEntry.Opaque(entry) -> (retrieveConstruction (Array.create 1 (retrieveOpaqueSurface entry)) None).Name
            | LibraryEntry.Composite(_) -> "COULDN'T BUILD CONSTRUCTION (SURFACE FOR COMPOSITE LIBRARY ENTRY)"
            | _ -> "INVALID MAPPING (OPAQUE TO NON-OPAQUE)"
        | ModelConstruction.Window(_) -> "COULDN'T BUILD CONSTRUCTION (THIRD-LEVEL WINDOW SURFACE)"
        | ModelConstruction.Composite(_, layers) ->
            let firstLayer = fst layers.[0]
            match firstLayer.MappingTarget with // hope it's symmetrical!
            | noMapping when noMapping = Unchecked.defaultof<LibraryEntry> -> "MISSING MAPPING FOR THIRD-LEVEL SURFACE"
            | LibraryEntry.Opaque(entry) -> (retrieveConstruction (Array.create 1 (retrieveOpaqueSurface entry)) None).Name
            | _ -> "BAD MAPPING FOR THIRD-LEVEL SURFACE"