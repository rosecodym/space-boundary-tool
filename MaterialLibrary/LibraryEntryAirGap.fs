namespace MaterialLibrary

open LibIdf.Base
open LibIdf.Idf

type LibraryEntryAirGap = {
    Name: FieldValue
    ThermalResistance: FieldValue
    }
    with
        static member internal Construct (obj:IdfObject) = {
            Name = obj.Fields.["Name"].Value
            ThermalResistance = obj.Fields.["Thermal Resistance"].Value
            }