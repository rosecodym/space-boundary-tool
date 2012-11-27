namespace ConstructionManagement.ModelConstructions

type ModelConstruction =
    | SingleOpaque of ModelMappingSource
    | Window of ModelMappingSource
    | LayerSet of string option * (ModelMappingSource * double) list
    with
        member public this.MappableComponents =
            match this with
            | SingleOpaque(src) -> Seq.singleton src
            | Window(src) -> Seq.singleton src
            | LayerSet(_, srcs) -> srcs |> Seq.map fst