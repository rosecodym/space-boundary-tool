namespace MaterialLibrary

open LibIdf.Idf

type LibraryEntry =
    | AirGap of LibraryEntryAirGap
    | Composite of string * (LibraryEntry array)
    | Gas of LibraryEntryGas
    | Glazing of LibraryEntryGlazing
    | Opaque of LibraryEntryOpaque
    with
        member this.Name =
            match this with
            | AirGap(properties) -> properties.Name.ToString()
            | Composite(name, _) -> name
            | Gas(properties) -> properties.Name.ToString()
            | Glazing(properties) -> properties.Name.ToString()
            | Opaque(properties) -> properties.Name.ToString()

        member this.IsForWindow =
            match this with
            | AirGap(_) -> false
            | Composite(_, layers) -> layers |> Seq.exists (fun layer -> not layer.IsForWindow) |> not
            | Gas(_) -> true
            | Glazing(_) -> true
            | Opaque(_) -> false

        member this.DisplayToUser =
            match this with
            | AirGap(_) -> false
            | Composite(_, layers) -> this.IsForWindow
            | Gas(_) -> false
            | Glazing(_) -> false
            | Opaque(_) -> true

        override this.ToString() = this.Name

        static member private CreateLayerMaybe obj =
            if obj = Unchecked.defaultof<IdfObject> then None
            else
                Some(LibraryEntry.Construct obj)

        static member Construct (obj:IdfObject) =
            match obj.Type with
            | "Construction" ->
                let name = obj.Name
                let layers = 
                    [|
                        "Outside Layer"
                        "Layer 2"
                        "Layer 3"
                        "Layer 4"
                        "Layer 5"
                        "Layer 6"
                        "Layer 7"
                        "Layer 8"
                        "Layer 9"
                        "Layer 10"
                    |]
                    |> Array.choose (fun fieldName -> LibraryEntry.CreateLayerMaybe obj.Fields.[fieldName].RefersTo)
                Composite(name, layers)
            | "Material" -> Opaque(LibraryEntryOpaque.Construct(obj))
            | "Material:AirGap" -> AirGap(LibraryEntryAirGap.Construct(obj))
            | "WindowMaterial:Gas" -> Gas(LibraryEntryGas.Construct(obj))
            | "WindowMaterial:Glazing" -> Glazing(LibraryEntryGlazing.Construct(obj))
            | _ -> failwith (sprintf "Tried to build a library entry out of an unknown type '%s'." obj.Type)