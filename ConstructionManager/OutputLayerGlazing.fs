namespace ConstructionManager

open LibIdf.Base
open MaterialLibrary

type internal OutputLayerGlazing (name, entry:MaterialLibrary.LibraryEntryGlazing, thickness) =
    inherit OutputLayer()
    override this.Name = name

    override this.AddToIdfV710(idf) =
        let obj = idf.CreateObject("WindowMaterial:Glazing")
        obj.Fields.["Name"].Value <- Alpha(this.Name)
        obj.Fields.["Optical Data Type"].Value <- entry.OpticalDataType
        obj.Fields.["Window Glass Spectral Data Set Name"].Value <- entry.WindowGlassSpectralDataSetName
        obj.Fields.["Thickness"].Value <- Real(thickness)
        obj.Fields.["Solar Transmittance at Normal Incidence"].Value <- entry.SolarTransmittanceAtNormalIncidence
        obj.Fields.["Front Side Solar Reflectance at Normal Incidence"].Value <- entry.FrontSideSolarReflectanceAtNormalIncidence
        obj.Fields.["Back Side Solar Reflectance at Normal Incidence"].Value <- entry.BackSideSolarReflectanceAtNormalIncidence
        obj.Fields.["Visible Transmittance at Normal Incidence"].Value <- entry.VisibleTransmittanceAtNormalIncidence
        obj.Fields.["Front Side Visible Reflectance at Normal Incidence"].Value <- entry.FrontSideVisibleReflectanceAtNormalIncidence
        obj.Fields.["Back Side Visible Reflectance at Normal Incidence"].Value <- entry.BackSideVisibleReflectanceAtNormalIncidence
        obj.Fields.["Infrared Transmittance at Normal Incidence"].Value <- entry.InfraredTransmittanceAtNormalIncidence
        obj.Fields.["Front Side Infrared Hemispherical Emissivity"].Value <- entry.FrontSideInfraredHemisphericalEmissivity
        obj.Fields.["Back Side Infrared Hemispherical Emissivity"].Value <- entry.BackSideInfraredHemisphericalEmissivity
        obj.Fields.["Conductivity"].Value <- entry.Conductivity
        obj.Fields.["Dirt Correction Factor for Solar and Visible Transmittance"].Value <- entry.DirtCorrectionFactorForSolarandVisibleTransmittance
        

