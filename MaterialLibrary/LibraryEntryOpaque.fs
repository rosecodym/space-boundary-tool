namespace MaterialLibrary

open LibIdf.Base
open LibIdf.Idf

type LibraryEntryOpaque = {
    Name: FieldValue
    Roughness: FieldValue
    Thickness: FieldValue
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
            Thickness = obj.Fields.["Thickness"].Value
            Conductivity = obj.Fields.["Conductivity"].Value
            Density = obj.Fields.["Density"].Value
            SpecificHeat = obj.Fields.["Specific Heat"].Value
            ThermalAbsorptance = obj.Fields.["Thermal Absorptance"].Value
            SolarAbsorptance = obj.Fields.["Solar Absorptance"].Value
            VisibleAbsorptance = obj.Fields.["Visible Absorptance"].Value
            }