namespace ConstructionManager

open System

open LibIdf.Idf

[<AbstractClass>]
type OutputLayer () =
    abstract Name : string with get

    abstract member AddToIdfV710 : Idf -> unit

    override this.Equals(obj:Object) = 
        match obj with
        | :? OutputLayer as layer -> this.Equals(layer)
        | _ -> false

    override this.GetHashCode() = this.Name.GetHashCode()

    interface IEquatable<OutputLayer> with
        member this.Equals(other) =
            if (Object.ReferenceEquals(other, null)) then false
            else if (Object.ReferenceEquals(this, other)) then true
            else if (this.GetType() <> other.GetType()) then false
            else this.Name = other.Name

    interface IComparable<OutputLayer> with
        member this.CompareTo(other) = this.Name.CompareTo(other.Name)

    interface IComparable with
        member this.CompareTo(obj) =
            match obj with
            | null -> 1
            | :? OutputLayer as layer -> (this :> IComparable<OutputLayer>).CompareTo(layer)
            | _ -> raise (ArgumentException())