namespace ConstructionManagement

open System
open System.Collections.Generic

[<AbstractClass>]
type OutputConstructionBase () =
    abstract Name : string with get
    abstract IsVirtual : bool with get

[<AbstractClass>]
type ProblemConstruction (msg) =
    inherit OutputConstructionBase ()
    member this.Message = msg

type UnorientedComposite () =
    inherit ProblemConstruction(
        "There was an attempt to generate a composite from an un-oriented \
         material layer set. This can happen if the original layer set was \
         improperly defined as a material list. The construction has been \
         named 'UNORIENTED COMPOSITE' in the IDF. You will have to create \
         this construction manually.")
    override this.Name = "UNORIENTED COMPOSITE"
    override this.IsVirtual = false

type UnknownProblemComposite () =
    inherit ProblemConstruction(
        "There was an unknown problem automatically generating a \
         construction. It has been assigned the construction name 'UNMAPPED'. \
         You will have to create this construction manually.")
    override this.Name = "UNMAPPED"
    override this.IsVirtual = false

type BadMappingConstruction () =
    inherit ProblemConstruction(
        "There was an attempt to automatically generate a composite for a \
         missing or invalid mapping target. It has been assigned the \
         construction name 'BAD MAPPING'. You will have to create this \
         construction manually.")
    override this.Name = "BAD MAPPING"
    override this.IsVirtual = false

type LibraryComposite () =
    inherit ProblemConstruction(
        "There was an attempt to automatically generate an opaque \
         construction from a composite library entry. This is not yet \
         supported. The construction has been named 'COMPOSITE LIBRARY \
         ENTRY'. You will have to create this construction manually.")
    override this.Name = "COMPOSITE LIBRARY ENTRY"
    override this.IsVirtual = false

type AdiabaticWindowConstruction () =
    inherit ProblemConstruction(
        "There was an attempt to generate an adiabatic surface construction \
         for a window library composite. The construction has been named 'BAD \
         WINDOW TARGET'. You will have to create this construction manually.")
    override this.Name = "BAD WINDOW TARGET"
    override this.IsVirtual = false

type OutputConstruction (layers:OutputLayer list,
                         humanReadableName:string option,
                         variantLookup,
                         ?maxNameLength: int) =
    inherit OutputConstructionBase()

    let mangledHName = 
        match humanReadableName with
        | None -> None
        | Some(hname) -> Some(hname.Replace(",", " |").Replace("!", "_"))

    let layerNames = 
        layers 
        |> Seq.map (fun layer -> if layer <> Unchecked.defaultof<OutputLayer> then layer.Name else "UNMAPPED MATERIAL")
        |> Seq.toArray
    let identifier = 
        match mangledHName with
        | Some(name) -> sprintf "%s!%s" name (String.concat "!" layerNames) // "!" is the separator because it can't appear in E+ names
        | None -> sprintf "!%s" (String.concat "!" layerNames)

    let layerNames = layers |> List.map (fun layer -> layer.Name)
    let idName = sprintf "Composite id %i" (identifier.GetHashCode())

    let maxNL = defaultArg maxNameLength 100

    let name =
        match mangledHName, layerNames with
        | _, [] ->
            failwith 
                "There was an attempt to generate a construction with no \
                 layers. Please report this internal SBT bug."
        | None, [layerName] when layerName.Length <= maxNL -> layerName
        | None, _ -> idName
        | Some(hname), _ when hname.Length <= maxNL -> hname
        | Some(hname), [layerName] when layerName.Length <= maxNL -> layerName
        | Some(hname), _ -> idName

    override this.Name =
        match variantLookup(this.Identifier) with
        | Some(v) -> sprintf "%s (variant %i)" name (v + 1)
        | None -> name
    override this.IsVirtual =
        not (layers |> Seq.exists (fun layer -> not layer.IsIRTransparent))
    member this.InvariantName = name
    member this.LayerNames = List<string>(layerNames)
    member this.Identifier = identifier
    member this.Warnings =
        let tooLong =
            match mangledHName with
            | Some(hname) when hname.Length > maxNL ->
                Some(sprintf
                    "The name of construction '%s' is too long. It will be \
                     named '%s' in the IDF."
                     humanReadableName.Value
                     name)
            | _ -> None
        let outsideAir =
            if 
                layers.Head.IsAirLayer ||
                (List.rev layers).Head.IsAirLayer then
                Some(sprintf "Construction '%s' has an outside air layer" name)
            else None
        Array.choose id [|tooLong; outsideAir|]

    override this.Equals(obj:Object) =
        match obj with
        | :? OutputConstruction as c -> this.Equals(c)
        | _ -> false

    override this.GetHashCode () = identifier.GetHashCode()

    interface IEquatable<OutputConstruction> with
        member this.Equals(other) =
            if (Object.ReferenceEquals(other, null)) then false
            else if (Object.ReferenceEquals(this, other)) then true
            else this.Identifier = other.Identifier

    interface IComparable<OutputConstruction> with
        member this.CompareTo(other) = this.Identifier.CompareTo(other.Identifier)

    interface IComparable with
        member this.CompareTo(obj) =
            match obj with
            | null -> 1
            | :? OutputConstruction as c -> (this :> IComparable<OutputConstruction>).CompareTo(c)
            | _ -> raise (ArgumentException())
