module internal ConstructionManagement.OutputPatterns

open MaterialLibrary
open ConstructionManagement.ModelConstructions

let (|Empty|OpaqueSingleOnly|MappedWindow|SingleComposite|UnmappedWindow|MixedSingleAndComposite|Unknown|) modelConstructions =
    match modelConstructions with
    | [||] -> Empty
    | [|Window(src)|] when src.MappingTarget = Unchecked.defaultof<LibraryEntry> -> UnmappedWindow
    | [|Window(src)|] ->
        match src.MappingTarget with
        | LibraryEntry.Composite(name, layers) -> MappedWindow(name, layers)
        | _ -> failwith "invalid window entry load"
    | [|Composite(srcs)|] ->
        SingleComposite(srcs |> Array.map (fun (src, thickness) -> (src.MappingTarget, thickness)))
    | _ ->
        let mapped = modelConstructions |> Array.collect (fun (modelLayer:ModelConstruction) -> modelLayer.MappableComponents |> Seq.map (fun src -> src.MappingTarget) |> Array.ofSeq)
        if mapped.Length <> modelConstructions.Length then MixedSingleAndComposite else
        let asOpaque = Array.map (fun m ->
            match m with
            | noMapping when noMapping = Unchecked.defaultof<LibraryEntry> -> Some(Unchecked.defaultof<LibraryEntryOpaque>)
            | Opaque(opaque) -> Some(opaque)
            | _ -> None) mapped
        if not (Array.exists (fun (maybe:LibraryEntryOpaque option) -> maybe.IsNone) asOpaque) then OpaqueSingleOnly(Array.choose id asOpaque) else
        Unknown