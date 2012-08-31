namespace ConstructionManager

open LibIdf.Base
open MaterialLibrary

type internal OutputLayerGas (name, entry:MaterialLibrary.LibraryEntryGas, thickness) =
    inherit OutputLayer()
    override this.Name = name

    override this.AddToIdfV710(idf) =
        let obj = idf.CreateObject("WindowMaterial:Gas")
        obj.Fields.["Name"].Value <- Alpha(this.Name)
        obj.Fields.["Gas Type"].Value <- entry.GasType
        obj.Fields.["Thickness"].Value <- Real(thickness)
        obj.Fields.["Conductivity Coefficient A"].Value <- entry.ConductivityCoefficientA
        obj.Fields.["Conductivity Coefficient B"].Value <- entry.ConductivityCoefficientB
        obj.Fields.["Viscosity Coefficient A"].Value <- entry.ViscosityCoefficientA
        obj.Fields.["Viscosity Coefficient B"].Value <- entry.ViscosityCoefficientB
        obj.Fields.["Specific Heat Coefficient A"].Value <- entry.SpecificHeatCoefficientA
        obj.Fields.["Specific Heat Coefficient B"].Value <- entry.SpecificHeatCoefficientB
        obj.Fields.["Molecular Weight"].Value <- entry.MolecularWeight