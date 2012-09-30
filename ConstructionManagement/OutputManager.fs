namespace ConstructionManagement

open System
open System.Collections.Generic

open MaterialLibrary

type LibraryEntry = ConstructionManagement.MaterialLibrary.LibraryEntry

type OutputManager () =
    let mutable layers = Set.empty
    let mutable constructions = Set.empty

    let retrieveConstruction materials =
        let newC = OutputConstruction(materials)
        constructions <- constructions.Add(newC)
        newC

    let retrieveLayer (layer:OutputLayer) =
        layers <- layers.Add(layer)
        layer

    let retrieveOpaqueLayer (libraryEntry:LibraryEntryOpaque) thickness =
        retrieveLayer (OutputLayerOpaque(sprintf "%s (%.3f)" (libraryEntry.Name.ToString()) thickness, libraryEntry, thickness))

    let retrieveOpaqueSurface (libraryEntry:LibraryEntryOpaque) =
        retrieveLayer (OutputLayerOpaque(sprintf "%s (surface only)" (libraryEntry.Name.ToString()), libraryEntry, 0.001))

    let retrieveExactCopy (libraryEntry:LibraryEntry) =
        match libraryEntry with
        | LibraryEntry.AirGap(props) -> retrieveLayer (OutputLayerAirGap(float props.ThermalResistance))
        | LibraryEntry.Composite(_) -> raise (ArgumentException())
        | LibraryEntry.Gas(props) -> retrieveLayer (OutputLayerGas(props.Name.ToString(), props, float props.Thickness))
        | LibraryEntry.Glazing(props) -> retrieveLayer (OutputLayerGlazing(props.Name.ToString(), props, float props.Thickness))
        | LibraryEntry.Opaque(props) -> retrieveOpaqueLayer props (float props.Thickness)

    member this.AllOutputLayers = layers :> IEnumerable<OutputLayer>
    member this.AllOutputConstructions = constructions :> IEnumerable<OutputConstruction>

    member this.ConstructionNameForLayers(constructions:IList<LibraryEntry>, thicknesses:IList<double>) = "NOT IMPLEMENTED YET"
    member this.ConstructionNameForSurface(c:LibraryEntry) = "NOT IMPLEMENTED YET"