namespace MaterialLibrary

open LibIdf.Idf

type LibraryEntry =
    | Composite of string * ((LibraryEntry * double) array)
    | Gas of LibraryEntryGas
    | Glazing of LibraryEntryGlazing
    | Opaque of LibraryEntryOpaque
    with
        member this.Name =
            match this with
            | Composite(name, _) -> name
            | Gas(properties) -> properties.Name.ToString()
            | Glazing(properties) -> properties.Name.ToString()
            | Opaque(properties) -> properties.Name.ToString()

        member this.IsForWindow =
            match this with
            | Composite(_, layers) -> layers |> Seq.map fst |> Seq.exists (fun layer -> not layer.IsForWindow) |> not
            | Gas(_) -> true
            | Glazing(_) -> true
            | Opaque(_) -> false

        member this.DisplayToUser =
            match this with
            | Composite(_, layers) -> this.IsForWindow
            | Gas(_) -> false
            | Glazing(_) -> false
            | Opaque(_) -> true

        override this.ToString() = this.Name

        static member private CreateLayerThicknessTupleMaybe obj =
            if obj = Unchecked.defaultof<IdfObject> then None
            else
                let layer = LibraryEntry.Construct obj
                let thickness = float obj.Fields.["Thickness"].Value
                Some(layer, thickness)

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
                    |> Array.choose (fun fieldName -> LibraryEntry.CreateLayerThicknessTupleMaybe obj.Fields.[fieldName].RefersTo)
                Composite(name, layers)
            | "Material" -> Opaque(LibraryEntryOpaque.Construct(obj))
            | "WindowMaterial:Gas" -> Gas(LibraryEntryGas.Construct(obj))
            | "WindowMaterial:Glazing" -> Glazing(LibraryEntryGlazing.Construct(obj))
            | _ -> failwith (sprintf "Tried to build a library entry out of an unknown type '%s'." obj.Type)