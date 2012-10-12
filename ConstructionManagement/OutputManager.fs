namespace ConstructionManagement

open System
open System.Collections.Generic

open MaterialLibrary
open ConstructionManagement.ModelConstructions
open OutputPatterns

type OutputManager (warnDelegate : Action<string>) =
    let warn = warnDelegate.Invoke
    let mutable layers = Set.empty
    let mutable constructions = Set.empty

    let retrieveConstruction materials humanReadableName =
        let newC = OutputConstruction(materials, humanReadableName)
        if not (constructions.Contains(newC)) then // constructions have referential transparency, but this check is needed so duplicate "outside air layer" warnings aren't emitted
            match newC.HasOutsideAirLayer, humanReadableName with
            | false, _ -> ()
            | true, Some(name) -> warn (sprintf "Construction '%s' has an outside air layer." name)
            | true, None -> warn (sprintf "An unnamed construction has an outside air layer.")
            constructions <- constructions.Add(newC) // ...and since we already checked for membership, we might as well use that result to not to a no-op add
        newC

    let retrieveLayer (layer:OutputLayer) =
        layers <- layers.Add(layer)
        layer

    let retrieveOpaqueLayer (libraryEntry:LibraryEntryOpaque) thickness =
        retrieveLayer (OutputLayerOpaque(sprintf "%s (%.3f)" (libraryEntry.Name.ToString()) thickness, libraryEntry, thickness))

    let retrieveOpaqueSurface (libraryEntry:LibraryEntryOpaque) =
        retrieveLayer (OutputLayerOpaque(sprintf "%s (surface only)" (libraryEntry.Name.ToString()), libraryEntry, 0.001))

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

    member this.ConstructionNameForLayers(constructions:IList<ModelConstruction>, thicknesses:IList<double>) =
        match List.ofSeq constructions with
        | Empty -> (retrieveConstruction ([retrieveLayer (OutputLayerInfraredTransparent())]) None).Name
        | MappedWindow(name, libraryLayers) -> (retrieveConstruction (libraryLayers |> List.map (retrieveCopy None)) (Some(name))).Name
        | SimpleOnly(infos) -> 
            let outputLayers = List.map2 retrieveCopy (thicknesses |> Seq.map (fun t -> Some(t)) |> List.ofSeq) infos
            (retrieveConstruction outputLayers None).Name
        | SingleComposite(name, layers) -> // if the thicknesses don't match, then too bad
            let outputLayers = layers |> List.map (fun (entry, thickness) -> retrieveCopy (Some(thickness)) entry)
            (retrieveConstruction outputLayers name).Name
        | UnmappedWindow -> "WINDOW WITH UNMAPPED CONSTRUCTION"
        | MixedSingleAndComposite -> "UNSUPPORTED MAPPING (MIXED SINGLE MATERIALS AND COMPOSITES)"
        | Unknown -> "UNSUPPORTED MAPPING (UNKNOWN)"

    member this.ConstructionNameForSurface(c:ModelConstruction) = 
        match c with
        | ModelConstruction.SingleOpaque(src) ->
            match src.MappingTarget with
            | noMapping when noMapping = Unchecked.defaultof<LibraryEntry> -> "MISSING MAPPING FOR THIRD-LEVEL SURFACE"
            | LibraryEntry.Opaque(entry) -> (retrieveConstruction [(retrieveOpaqueSurface entry)] None).Name
            | LibraryEntry.Composite(_) -> "COULDN'T BUILD CONSTRUCTION (SURFACE FOR COMPOSITE LIBRARY ENTRY)"
            | _ -> "INVALID MAPPING (OPAQUE TO NON-OPAQUE)"
        | ModelConstruction.Window(_) -> "COULDN'T BUILD CONSTRUCTION (THIRD-LEVEL WINDOW SURFACE)"
        | ModelConstruction.Composite(_, layers) ->
            let firstLayer = fst layers.[0]
            match firstLayer.MappingTarget with // hope it's symmetrical!
            | noMapping when noMapping = Unchecked.defaultof<LibraryEntry> -> "MISSING MAPPING FOR THIRD-LEVEL SURFACE"
            | LibraryEntry.Opaque(entry) -> (retrieveConstruction [retrieveOpaqueSurface entry] None).Name
            | entry -> (retrieveConstruction [retrieveCopy None entry] None).Name
            | _ -> "BAD MAPPING FOR THIRD-LEVEL SURFACE"