namespace ConstructionManagement

open System
open System.Collections.Generic

type OutputConstruction (layers:OutputLayer list,
                         humanReadableName:string option,
                         warn:string -> unit,
                         ?maxNameLength: int) =
    let layerNames = 
        layers 
        |> Seq.map (fun layer -> if layer <> Unchecked.defaultof<OutputLayer> then layer.Name else "UNMAPPED MATERIAL")
        |> Seq.toArray
    let identifier = 
        match humanReadableName with
        | Some(name) -> sprintf "%s!%s" name (String.concat "!" layerNames) // "!" is the separator because it can't appear in E+ names
        | None -> sprintf "!%s" (String.concat "!" layerNames)

    let layerNames = layers |> List.map (fun layer -> layer.Name)
    let idName = sprintf "Composite id %i" (identifier.GetHashCode())

    let maxNL = defaultArg maxNameLength 100

    let name =
        match humanReadableName, layerNames with
        | _, [] ->
            failwith 
                "There was an attempt to generate a construction with no \
                 layers. Please report this internal SBT bug."
        | None, [layerName] when layerName.Length <= maxNL -> layerName
        | None, _ -> idName
        | Some(hname), _ when hname.Length <= maxNL -> hname
        | Some(hname), [layerName] when layerName.Length <= maxNL -> layerName
        | Some(hname), _ -> idName

    do
        match humanReadableName with
        | Some(hname) when hname.Length > maxNL ->
            warn (sprintf
                    "The name of construction '%s' is too long. It will be \
                     named '%s' in the IDF."
                     hname
                     name)
        | _ -> ()
        if match layers, List.rev layers with
            | outside :: rest, _ when outside.IsAirLayer -> true
            | _, outside :: rest when outside.IsAirLayer -> true
            | _ -> false
        then
            warn (sprintf "Construction '%s' has an outside air layer" name)

    member this.Name = name
    member this.LayerNames = List<string>(layerNames)
    member this.Identifier = identifier

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
