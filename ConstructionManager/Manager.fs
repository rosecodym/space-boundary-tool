namespace ConstructionManager

open System
open System.Collections.Generic

open MaterialLibrary

open LayerPatterns

type Manager (sbtMaterialIDToLibraryEntry:Func<int, MaterialLibrary.LibraryEntry>) =
    let lookupLibraryEntry = sbtMaterialIDToLibraryEntry.Invoke
    let mutable allMaterials = Set.empty
    let mutable allConstructions = Set.empty

    let retrieveConstruction materials =
        let newC = Construction(materials)
        allConstructions <- Set.add newC allConstructions
        newC

    let retrieveLayer (layer:OutputLayer) =
        allMaterials <- Set.add layer allMaterials
        layer

    let retrieveOpaqueLayer (libraryEntry:LibraryEntryOpaque) thickness =
        retrieveLayer (OutputLayerOpaque(sprintf "%s (%.3f)" (libraryEntry.Name.ToString()) thickness, libraryEntry, thickness))

    let retrieveOpaqueSurface (libraryEntry:LibraryEntryOpaque) =
        retrieveLayer (OutputLayerOpaque(sprintf "%s (surface only)" (libraryEntry.Name.ToString()), libraryEntry, 0.001))

    let retrieveExactCopy (libraryEntry:LibraryEntry) =
        match libraryEntry with
        | AirGap(props) -> retrieveLayer (OutputLayerAirGap(float props.ThermalResistance))
        | Composite(_) -> raise (ArgumentException())
        | Gas(props) -> retrieveLayer (OutputLayerGas(props.Name.ToString(), props, float props.Thickness))
        | Glazing(props) -> retrieveLayer (OutputLayerGlazing(props.Name.ToString(), props, float props.Thickness))
        | Opaque(props) -> retrieveOpaqueLayer props (float props.Thickness)

    member this.AllMaterials = allMaterials :> IEnumerable<OutputLayer>
    member this.AllConstructions = allConstructions :> IEnumerable<Construction>

    member this.ConstructionNameForLayers(materials:IList<Sbt.CoreTypes.MaterialLayer>) =
        let ids, thicknesses = Array.unzip(materials |> Seq.map (fun sbtLayer -> (sbtLayer.Id, sbtLayer.Thickness)) |> Seq.toArray)
        let asLibraryEntries = ids |> Array.map lookupLibraryEntry
        match asLibraryEntries with
        | OpaqueOnly(opaqueEntries) ->
            let outputLayers =
                (Array.zip opaqueEntries thicknesses)
                |> Array.map (fun (entry, thickness) -> retrieveOpaqueLayer entry thickness)
            (retrieveConstruction outputLayers).Name
        | SingleComposite(_, innerLayers) -> (retrieveConstruction (innerLayers |> Array.map retrieveExactCopy)).Name
        | _ -> "UNMAPPED CONSTRUCTION"

    member this.ConstructionNameForSurface(id) =
        match lookupLibraryEntry id with
        | Opaque(entry) -> retrieveConstruction(Array.create 1 (retrieveOpaqueSurface entry)).Name
        | _ -> "BAD SURFACE CONSTRUCTION"

