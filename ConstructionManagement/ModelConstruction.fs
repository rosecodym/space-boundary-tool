namespace ConstructionManagement.ModelConstructions

type ModelConstruction =
    | SingleOpaque of ModelMappingSource
    | Window of ModelMappingSource
    | Composite of string option * (ModelMappingSource * double) list
    with
        member public this.MappableComponents =
            match this with
            | SingleOpaque(src) -> Seq.singleton src
            | Window(src) -> Seq.singleton src
            | Composite(_, srcs) -> srcs |> Seq.map fst
        member public this.SetComponentUsages(usage:ModelConstructionUsage) =
            match usage, this with
            | u, SingleOpaque(src) -> src.AddUsage u
            | u, Window(src) -> src.AddUsage u
            | ModelConstructionUsage.Surface, Composite(_, (firstLayer, _) :: _) -> firstLayer.AddUsage ModelConstructionUsage.Surface
            | u, Composite(_, layers) -> layers |> Seq.iter (fun (layer, _) -> layer.AddUsage u)