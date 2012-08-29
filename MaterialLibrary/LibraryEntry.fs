namespace MaterialLibrary

open LibIdf.Base
open LibIdf.Idf

type LibraryEntryOpaque = {
    Name: FieldValue
    Roughness: FieldValue
    Conductivity: FieldValue
    Density: FieldValue
    SpecificHeat: FieldValue
    ThermalAbsorptance: FieldValue
    SolarAbsorptance: FieldValue
    VisibleAbsorptance: FieldValue
    }
    with
        static member internal Construct (obj:IdfObject) = {
            Name = obj.Fields.["Name"].Value
            Roughness = obj.Fields.["Roughness"].Value
            Conductivity = obj.Fields.["Conductivity"].Value
            Density = obj.Fields.["Density"].Value
            SpecificHeat = obj.Fields.["SpecificHeat"].Value
            ThermalAbsorptance = obj.Fields.["Thermal Absorptance"].Value
            SolarAbsorptance = obj.Fields.["Solar Absorptance"].Value
            VisibleAbsorptance = obj.Fields.["Visible Absorptance"].Value
            }

type LibraryEntry =
    | Opaque of LibraryEntryOpaque
    with
        member this.Name =
            match this with
            | Opaque(properties) -> properties.Name.ToString()
        static member Construct (obj:IdfObject) =
            match obj.Type with
            | "Material" -> Opaque(LibraryEntryOpaque.Construct(obj))
            | _ -> failwith "Tried to build a library entry out of an unknown type."