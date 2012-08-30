namespace ConstructionManager

open LibIdf.Base
open MaterialLibrary

type internal OutputLayerOpaque (name, libraryEntry:MaterialLibrary.LibraryEntryOpaque, thickness) =
    inherit OutputLayer()
    override this.Name = sprintf "%s (%.3f)" name thickness

    override this.AddToIdfV710(idf) =
        let obj = idf.CreateObject("Material")
        obj.Fields.["Name"].Value <- Alpha(this.Name)
        obj.Fields.["Thickness"].Value <- Real(thickness)
        obj.Fields.["Roughness"].Value <- libraryEntry.Roughness
        obj.Fields.["Conductivity"].Value <- libraryEntry.Conductivity
        obj.Fields.["Density"].Value <- libraryEntry.Density
        obj.Fields.["Specific Heat"].Value <- libraryEntry.SpecificHeat
        obj.Fields.["Thermal Absorptance"].Value <- libraryEntry.ThermalAbsorptance;
        obj.Fields.["Solar Absorptance"].Value <- libraryEntry.SolarAbsorptance;
        obj.Fields.["Visible Absorptance"].Value <- libraryEntry.VisibleAbsorptance;

