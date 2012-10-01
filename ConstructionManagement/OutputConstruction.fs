namespace ConstructionManagement

open System
open System.Collections.Generic

type OutputConstruction (layers:OutputLayer array, humanReadableName:string option) =
    let layerNames = 
        layers 
        |> Seq.map (fun layer -> if layer <> Unchecked.defaultof<OutputLayer> then layer.Name else "UNMAPPED MATERIAL")
        |> Seq.toArray
    let identifier = String.concat "!" layerNames // "!" is the separator because it can't appear in E+ names

    member this.Name =
        match humanReadableName, layerNames with
        | Some(name), _ -> name
        | None, [|layerName|] -> layerName
        | _ -> sprintf "Unnamed composite (id %i)" (this.GetHashCode())

    member this.LayerNames = List<string>(layerNames)

    member private this.Identifier = identifier

    override this.Equals(obj:Object) =
        match obj with
        | :? OutputConstruction as c -> this.Equals(c)
        | _ -> false

    override this.GetHashCode () = this.Identifier.GetHashCode()

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
