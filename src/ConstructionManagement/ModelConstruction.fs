namespace ConstructionManagement.ModelConstructions

type ModelConstruction =
    | SingleOpaque of ModelMappingSource
    | Window of ModelMappingSource
    | LayerSet of string option * (ModelMappingSource * double) list
    with
        member this.MappableComponents =
            match this with
            | SingleOpaque(src) -> Seq.singleton src
            | Window(src) -> Seq.singleton src
            | LayerSet(_, srcs) -> srcs |> Seq.map fst
        static member normalsParallel a b =
            let dotproduct (ax, ay, az) (bx, by, bz) =
                ax * bx + ay * by + az * bz
            let magSq d = dotproduct d d
            let magnitude = magSq >> sqrt
            let lhs = dotproduct a b 
            let rhs = (magnitude a) * (magnitude b)
            abs(lhs - rhs) < 0.001  || abs(lhs + rhs) < 0.001 // magic eps
        static member normalsAntiparallel a b =
            let dotproduct (ax, ay, az) (bx, by, bz) =
                ax * bx + ay * by + az * bz
            let magSq d = dotproduct d d
            let (ax: float), (ay: float), (az: float) = a
            let bx, by, bz = b
            let sum = ax + bx, ay + by, az + bz
            magSq sum < magSq a + magSq b
            