namespace MaterialLibrary

open LibIdf.Base
open LibIdf.Idf

type LibraryEntryGlazing = {
    Name: FieldValue
    OpticalDataType: FieldValue
    WindowGlassSpectralDataSetName: FieldValue
    Thickness: FieldValue
    SolarTransmittanceAtNormalIncidence: FieldValue
    FrontSideSolarReflectanceAtNormalIncidence: FieldValue
    BackSideSolarReflectanceAtNormalIncidence: FieldValue
    VisibleTransmittanceAtNormalIncidence: FieldValue
    FrontSideVisibleReflectanceAtNormalIncidence: FieldValue
    BackSideVisibleReflectanceAtNormalIncidence: FieldValue
    InfraredTransmittanceAtNormalIncidence: FieldValue
    FrontSideInfraredHemisphericalEmissivity: FieldValue
    BackSideInfraredHemisphericalEmissivity: FieldValue
    Conductivity: FieldValue
    DirtCorrectionFactorForSolarandVisibleTransmittance: FieldValue
    }
    with
        static member internal Construct (obj:IdfObject) = {
            Name = obj.Fields.["Name"].Value
            OpticalDataType = obj.Fields.["Optical Data Type"].Value
            WindowGlassSpectralDataSetName = obj.Fields.["Window Glass Spectral Data Set Name"].Value
            Thickness = obj.Fields.["Thickness"].Value
            SolarTransmittanceAtNormalIncidence = obj.Fields.["Solar Transmittance at Normal Incidence"].Value
            FrontSideSolarReflectanceAtNormalIncidence = obj.Fields.["Front Side Solar Reflectance at Normal Incidence"].Value
            BackSideSolarReflectanceAtNormalIncidence = obj.Fields.["Back Side Solar Reflectance at Normal Incidence"].Value
            VisibleTransmittanceAtNormalIncidence = obj.Fields.["Visible Transmittance at Normal Incidence"].Value
            FrontSideVisibleReflectanceAtNormalIncidence = obj.Fields.["Front Side Visible Reflectance at Normal Incidence"].Value
            BackSideVisibleReflectanceAtNormalIncidence = obj.Fields.["Back Side Visible Reflectance at Normal Incidence"].Value
            InfraredTransmittanceAtNormalIncidence = obj.Fields.["Infrared Transmittance at Normal Incidence"].Value
            FrontSideInfraredHemisphericalEmissivity = obj.Fields.["Front Side Infrared Hemispherical Emissivity"].Value
            BackSideInfraredHemisphericalEmissivity = obj.Fields.["Back Side Infrared Hemispherical Emissivity"].Value
            Conductivity = obj.Fields.["Conductivity"].Value
            DirtCorrectionFactorForSolarandVisibleTransmittance = obj.Fields.["Dirt Correction Factor for Solar and Visible Transmittance"].Value
            }