namespace MaterialLibrary

open LibIdf.Base
open LibIdf.Idf

type LibraryEntryGas = {
    Name: FieldValue
    GasType: FieldValue
    ConductivityCoefficientA: FieldValue
    ConductivityCoefficientB: FieldValue
    ViscosityCoefficientA: FieldValue
    ViscosityCoefficientB: FieldValue
    SpecificHeatCoefficientA: FieldValue
    SpecificHeatCoefficientB: FieldValue
    MolecularWeight: FieldValue
    }
    with
        static member internal Construct (obj:IdfObject) = {
            Name = obj.Fields.["Name"].Value
            GasType = obj.Fields.["Gas Type"].Value
            ConductivityCoefficientA = obj.Fields.["Conductivity Coefficient A"].Value
            ConductivityCoefficientB = obj.Fields.["Conductivity Coefficient B"].Value
            ViscosityCoefficientA = obj.Fields.["Viscosity Coefficient A"].Value
            ViscosityCoefficientB = obj.Fields.["Viscosity Coefficient B"].Value
            SpecificHeatCoefficientA = obj.Fields.["Specific Heat Coefficient A"].Value
            SpecificHeatCoefficientB = obj.Fields.["Specific Heat Coefficient B"].Value
            MolecularWeight = obj.Fields.["Molecular Weight"].Value
            }