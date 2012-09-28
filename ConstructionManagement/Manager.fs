namespace ConstructionManagement

open System
open System.Collections.Generic

open LibIdf.Idf
open Sbt

type Manager () =
    let mutable currentLibrary = Set.empty
    let mutable outputLayers = Set.empty
    let mutable outputConstructions = Set.empty
    let currentModelMappingSources = Dictionary<string, ModelMappingSource>()

    let retrieveOutputConstruction materials =
        let newC = OutputConstruction(materials)
        outputConstructions <- Set.add newC outputConstructions
        newC

    let retrieveLayer (layer:OutputLayer) =
        outputLayers <- Set.add layer outputLayers
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

    member this.LibraryEntries = currentLibrary |> Set.toSeq
    member this.ModelMappingSources = currentModelMappingSources.Values :> seq<ModelMappingSource>
    member this.AllOutputLayers = outputLayers :> seq<OutputLayer>
    member this.AllOutputConstructions = outputConstructions :> seq<OutputConstruction>

    member this.LoadMaterialLibrary(idf:Idf, notify:Action<string>) =
        currentLibrary <-
            Seq.append (idf.GetObjectsByType("Material", false)) (idf.GetObjectsByType("Construction", false))
            |> Seq.choose (fun obj ->
                try Some(LibraryEntry.Construct(obj))
                with | _ ->
                    let objName = if String.IsNullOrWhiteSpace(obj.Name) then "<unnamed-object>" else obj.Name
                    notify.Invoke(sprintf "Warning: Failed to load material library object '%s'. Check the definition in the IDF.\n" objName)
                    None)
            |> Set.ofSeq

    member this.GetModelConstructionSingleOpaque(name:string) = 
        let exists = ref Unchecked.defaultof<ModelMappingSource>
        if not (currentModelMappingSources.TryGetValue(name, exists))
        then exists := ModelMappingSource(name, false)
        SingleOpaque(!exists)

    member this.GetModelConstructionWindow(name:string) =
        let exists = ref Unchecked.defaultof<ModelMappingSource>
        if not (currentModelMappingSources.TryGetValue(name, exists))
        then exists := ModelMappingSource(name, true)
        Window(!exists)

    member this.GetModelConstructionComposite(names:IList<string>, thicknesses:IList<double>) =
        let mappables = 
            names 
            |> Seq.map (fun name ->
                let exists = ref Unchecked.defaultof<ModelMappingSource>
                if not (currentModelMappingSources.TryGetValue(name, exists))
                then exists := ModelMappingSource(name, false)
                !exists)
            |> Array.ofSeq
        ModelConstruction.Composite(Array.zip mappables (Array.ofSeq thicknesses))

    member this.ConstructionNameForLayers(materials:IList<Sbt.CoreTypes.MaterialLayer>, constructionLookup:Func<int, ModelConstruction>) = "NOT IMPLEMENTED YET"
    member this.ConstructionNameForSurface(id:int, constructionLookup:Func<int, ModelConstruction>) = "NOT IMPLEMENTED YET"

    member this.ClearModelConstructions() = currentModelMappingSources.Clear()