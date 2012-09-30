namespace ConstructionManagement.ModelConstructions

type ModelConstruction =
    | SingleOpaque of ModelMappingSource
    | Window of ModelMappingSource
    | Composite of (ModelMappingSource * double) array
    with
        member public this.MappableComponents =
            match this with
            | SingleOpaque(src) -> Seq.singleton src
            | Window(src) -> Seq.singleton src
            | Composite(srcs) -> srcs |> Seq.map fst