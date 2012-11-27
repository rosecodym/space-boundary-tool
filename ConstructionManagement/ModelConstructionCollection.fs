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

    member this.GetModelConstructionLayerSet(compositeName:string, 
                                             names:IList<string>, 
                                             thicknesses:IList<double>) =
        let single = this.GetModelConstructionSingleOpaque
        if names.Count = 1 then single names.[0] else
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
        let layers = List.zip mappables (List.ofSeq thicknesses)
        if compositeName <> Unchecked.defaultof<string>
            then LayerSet(Some(compositeName), layers)
            else LayerSet(None, layers)
