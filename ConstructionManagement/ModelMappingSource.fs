namespace ConstructionManagement.ModelConstructions

open System
open System.ComponentModel

open ConstructionManagement.MaterialLibrary

type LibraryEntry = ConstructionManagement.MaterialLibrary.LibraryEntry

type ModelMappingSource (name:string, forWindows:bool) =
    let mutable mappedTo = None

    // this is part of the magic spell to implement INotifyPropertyChanged
    let propertyChanged = Event<_, _>()

    member this.Name = name
    member this.IsForWindows = forWindows
    member this.MappingTarget
        with get () = if mappedTo.IsNone then Unchecked.defaultof<LibraryEntry> else mappedTo.Value
        and set (value) = 
            if value <> Unchecked.defaultof<LibraryEntry> then mappedTo <- Some(value) else mappedTo <- None
            propertyChanged.Trigger(this, PropertyChangedEventArgs("MappingTarget"))

    override this.Equals(obj:Object) = 
        match obj with
        | :? ModelMappingSource as other -> (this :> IEquatable<ModelMappingSource>).Equals(other)
        | _ -> false

    override this.GetHashCode() = this.Name.GetHashCode()

    interface IEquatable<ModelMappingSource> with
        member this.Equals(other) =
            if (Object.ReferenceEquals(other, null)) then false
            else if (Object.ReferenceEquals(this, other)) then true
            else if (this.GetType() <> other.GetType()) then false
            else this.Name = other.Name

    interface IComparable<ModelMappingSource> with
        member this.CompareTo(other) = this.Name.CompareTo(other.Name)

    interface IComparable with
        member this.CompareTo(obj) =
            match obj with
            | null -> 1
            | :? ModelMappingSource as other -> (this :> IComparable<ModelMappingSource>).CompareTo(other)
            | _ -> raise (ArgumentException())

    interface INotifyPropertyChanged with
        [<CLIEvent>]
        member this.PropertyChanged = propertyChanged.Publish
