module internal ConstructionManagement.OutputPatterns

open MaterialLibrary
open ConstructionManagement.ModelConstructions

let (|Empty|SimpleOnly|MappedWindow|SingleComposite|UnmappedWindow|MixedSingleAndComposite|Unknown|) modelConstructions =
    match modelConstructions with
    | [] -> Empty
    | [Window(src)] when src.MappingTarget = Unchecked.defaultof<LibraryEntry> -> UnmappedWindow
    | [Window(src)] ->
        match src.MappingTarget with
        | LibraryEntry.Composite(name, layers) -> MappedWindow(name, layers)
        | _ -> failwith "invalid window entry load"
    | [LayerSet(name, srcs)] -> 
        let getTarget = 
            fun (src:ModelMappingSource, thickness) -> 
                (src.MappingTarget, thickness)
        SingleComposite(name, srcs |> List.map getTarget)
    | _ ->
        let mapped = 
            modelConstructions 
            |> Seq.collect (fun (modelLayer:ModelConstruction) -> modelLayer.MappableComponents |> Seq.map (fun src -> src.MappingTarget))
            |> List.ofSeq
        if mapped.Length <> modelConstructions.Length then MixedSingleAndComposite else
        let asSimple = List.map (fun m ->
            match m with
            | noMapping when noMapping = Unchecked.defaultof<LibraryEntry> -> Some(Unchecked.defaultof<LibraryEntry>)
            | AirGap(_) | NoMass(_) | Opaque(_) -> Some(m)
            | _ -> None) mapped
        if not (List.exists (fun (maybe:LibraryEntry option) -> maybe.IsNone) asSimple) then SimpleOnly(List.choose id asSimple) else
        Unknown