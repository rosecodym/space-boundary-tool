module internal ConstructionManagement.OutputPatterns

open MaterialLibrary
open ConstructionManagement.ModelConstructions

let (|Empty|_|) = function
    | [] -> Some(Empty)
    | _ -> None
    
let (|MappedWindow|_|) = function
    | [Window(src)] ->
        match src.MappingTarget with
        | LibraryEntry.Composite(name, layers) -> 
            Some(MappedWindow(name, layers))
        | _ -> failwith "impossible! invalid window entry load"
    | _ -> None

let (|SingleAirSpace|_|) = function
    | [SingleOpaque(src)] ->
        match src.MappingTarget with
        | AirGap(_) -> Some(())
        | _ -> None
    | _ -> None

let (|SingleComposite|_|) = function
    | [LayerSet(name, srcs)] ->
        let sources, thicknesses = List.unzip srcs
        let fullySymmetrical =
            sources = List.rev sources && thicknesses = List.rev thicknesses
        let identicalOutsideSources =
            List.head sources = List.head (List.rev sources)
        let targets = sources |> List.map (fun s -> s.MappingTarget)
        Some(SingleComposite(name, List.zip targets thicknesses))
    | _ -> None

let (|IdenticalOutsideSourcesComposite|_|) = function
    | [LayerSet(name, srcs)] ->
        let sources, thicknesses = List.unzip srcs
        if List.head sources = List.head (List.rev sources) then
            let targets = sources |> List.map (fun s -> s.MappingTarget)
            let targeted = List.zip targets thicknesses
            Some(IdenticalOutsideSourcesComposite(name,targeted))
        else None
    | _ -> None

let (|SimpleOnly|_|) (modelConstructions: ModelConstruction list) =
    let mapped =
        let getTarget = fun (src: ModelMappingSource) -> src.MappingTarget
        let getTargets = fun (modelLayer: ModelConstruction) -> 
            modelLayer.MappableComponents |> Seq.map getTarget
        modelConstructions |> Seq.collect getTargets |> List.ofSeq
    if mapped.Length = modelConstructions.Length then
        let asSimple = mapped |> List.map (fun m -> 
            match m with
            | noMapping when noMapping = Unchecked.defaultof<LibraryEntry> ->
                Some(Unchecked.defaultof<LibraryEntry>)
            | AirGap(_) | NoMass(_) | Opaque(_) -> Some(m)
            | _ -> None)
        if not (asSimple |> List.exists (fun maybe -> maybe.IsNone)) then
            Some(SimpleOnly(List.choose id asSimple))
        else None
    else None

let (|TwoComposites|_|) snorm cnorms = function
    | [LayerSet(firstName, firstLayers); LayerSet(secondName, secondLayers)] ->
        let map (layers: (ModelMappingSource * double) list) cnorm =
            let srcs, thicknesses = List.unzip layers
            let mapped = srcs |> Seq.map (fun src -> src.MappingTarget)
            let res = List.zip (List.ofSeq mapped) thicknesses
            if ModelConstruction.normalsAntiparallel snorm cnorm
                then List.rev res else res
        let cnorm1, cnorm2 =
            match cnorms with
            | Some(a) :: Some(b) :: _ -> a, b
            | _ -> failwith "improper cnorms list"
        let name1 = defaultArg firstName "Unnamed composite"
        let name2 = defaultArg secondName "unnamed composite"
        Some(TwoComposites(name1 + "+" + name2,
                           map firstLayers cnorm1, 
                           map secondLayers cnorm2))
    | _ -> None
