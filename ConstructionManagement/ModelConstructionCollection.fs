namespace ConstructionManagement.ModelConstructions

open System
open System.Collections.Generic

type ModelConstructionCollection () =
    let currentModelMappingSources = Dictionary<string, ModelMappingSource>()

    member this.MappingSources = currentModelMappingSources.Values :> ICollection<ModelMappingSource>
    
    member this.GetModelConstructionSingleOpaque(name:string) = 
        let exists = ref Unchecked.defaultof<ModelMappingSource>
        if not (currentModelMappingSources.TryGetValue(name, exists)) then 
            exists := ModelMappingSource(name, false)
            currentModelMappingSources.[name] <- !exists
        SingleOpaque(!exists)

    member this.GetModelConstructionWindow(name:string) =
        let exists = ref Unchecked.defaultof<ModelMappingSource>
        if not (currentModelMappingSources.TryGetValue(name, exists)) then 
            exists := ModelMappingSource(name, true)
            currentModelMappingSources.[name] <- !exists
        Window(!exists)

    member this.GetModelConstructionComposite(compositeName:string, names:IList<string>, thicknesses:IList<double>) =
        if names.Count = 1 then this.GetModelConstructionSingleOpaque(names.[0]) else
        let mappables = 
            names 
            |> Seq.map (fun name ->
                let exists = ref Unchecked.defaultof<ModelMappingSource>
                if not (currentModelMappingSources.TryGetValue(name, exists))
                    then
                        exists := ModelMappingSource(name, false)
                        currentModelMappingSources.[name] <- !exists
                !exists)
            |> List.ofSeq
        if compositeName <> Unchecked.defaultof<string>
            then ModelConstruction.Composite(Some(compositeName), List.zip mappables (List.ofSeq thicknesses))
            else ModelConstruction.Composite(None, List.zip mappables (List.ofSeq thicknesses))