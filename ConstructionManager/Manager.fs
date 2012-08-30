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

    let retrieveOpaqueLayer (libraryEntry:LibraryEntryOpaque) thickness =
        let newLayer = OutputLayerOpaque(sprintf "%s (%.3f)" (libraryEntry.Name.ToString()) thickness, libraryEntry, thickness) :> OutputLayer
        allMaterials <- Set.add newLayer allMaterials
        newLayer

    let retrieveOpaqueSurface (libraryEntry:LibraryEntryOpaque) =
        let newLayer = OutputLayerOpaque(sprintf "%s (surface only)" (libraryEntry.Name.ToString()), libraryEntry, 0.001) :> OutputLayer
        allMaterials <- Set.add newLayer allMaterials
        newLayer

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
        | _ -> "UNMAPPED CONSTRUCTION"

    member this.ConstructionNameForSurface(id) =
        match lookupLibraryEntry id with
        | Opaque(entry) -> retrieveConstruction(Array.create 1 (retrieveOpaqueSurface entry)).Name

