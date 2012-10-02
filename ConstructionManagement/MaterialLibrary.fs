module ConstructionManagement.MaterialLibrary

open System
open System.Collections.Generic

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

type LibraryEntryGas = {
    Name: FieldValue
    GasType: FieldValue
    Thickness: FieldValue
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
            Thickness = obj.Fields.["Thickness"].Value
            ConductivityCoefficientA = obj.Fields.["Conductivity Coefficient A"].Value
            ConductivityCoefficientB = obj.Fields.["Conductivity Coefficient B"].Value
            ViscosityCoefficientA = obj.Fields.["Viscosity Coefficient A"].Value
            ViscosityCoefficientB = obj.Fields.["Viscosity Coefficient B"].Value
            SpecificHeatCoefficientA = obj.Fields.["Specific Heat Coefficient A"].Value
            SpecificHeatCoefficientB = obj.Fields.["Specific Heat Coefficient B"].Value
            MolecularWeight = obj.Fields.["Molecular Weight"].Value
            }

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

type LibraryEntryNoMass = {
    Name: FieldValue
    Roughness: FieldValue
    ThermalResistance: FieldValue
    }
    with
        static member internal Construct (obj:IdfObject) = {
            Name = obj.Fields.["Name"].Value
            Roughness = obj.Fields.["Roughness"].Value
            ThermalResistance = obj.Fields.["Thermal Resistance"].Value
            }

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

type LibraryEntryType =
    | AirGap = 0
    | Composite = 1
    | Gas = 2
    | Glazing = 3
    | NoMass = 5
    | Opaque = 4

type LibraryEntry =
    | AirGap of LibraryEntryAirGap
    | Composite of string * (LibraryEntry array)
    | Gas of LibraryEntryGas
    | Glazing of LibraryEntryGlazing
    | NoMass of LibraryEntryNoMass
    | Opaque of LibraryEntryOpaque
    with
        member this.Name =
            match this with
            | AirGap(properties) -> properties.Name.ToString()
            | Composite(name, _) -> name
            | Gas(properties) -> properties.Name.ToString()
            | Glazing(properties) -> properties.Name.ToString()
            | NoMass(properties) -> properties.Name.ToString()
            | Opaque(properties) -> properties.Name.ToString()

        member this.Type =
            match this with
            | AirGap(_) -> LibraryEntryType.AirGap
            | Composite(_) -> LibraryEntryType.Composite
            | Gas(_) -> LibraryEntryType.Gas
            | Glazing(_) -> LibraryEntryType.Glazing
            | NoMass(_) -> LibraryEntryType.NoMass
            | Opaque(_) -> LibraryEntryType.Opaque

        member this.IsForWindows =
            match this with
            | AirGap(_) | NoMass(_) | Opaque(_) -> false
            | Gas(_) | Glazing(_) -> true
            | Composite(_, layers) -> layers |> Seq.exists (fun layer -> not layer.IsForWindows) |> not

        override this.ToString() = this.Name

        static member private CreateLayerMaybe obj =
            if obj = Unchecked.defaultof<IdfObject>
            then None
            else Some(LibraryEntry.Construct obj)

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
            | "Material:NoMass" -> NoMass(LibraryEntryNoMass.Construct(obj))
            | "Material:AirGap" -> AirGap(LibraryEntryAirGap.Construct(obj))
            | "WindowMaterial:Gas" -> Gas(LibraryEntryGas.Construct(obj))
            | "WindowMaterial:Glazing" -> Glazing(LibraryEntryGlazing.Construct(obj))
            | _ -> failwith (sprintf "Tried to build a library entry out of an unknown type '%s'." obj.Type)

let Load(idf:Idf, notify:Action<string>) : ISet<LibraryEntry> =
    let compositeIsWindow = (fun (c:IdfObject) -> c.Fields |> Seq.exists (fun f -> f.RefersTo <> Unchecked.defaultof<IdfObject> && (f.RefersTo.Type = "WindowMaterial:Gas" || f.RefersTo.Type = "WindowMaterial:Glazing")))
    let groups = [
        idf.GetObjectsByType("Material", false) :> seq<IdfObject>
        idf.GetObjectsByType("Material:NoMass", false) :> seq<IdfObject>
        idf.GetObjectsByType("Material:AirGap", false) :> seq<IdfObject>
        idf.GetObjectsByType("Construction", false) |> Seq.filter compositeIsWindow
        ]
    let entries =
        groups
        |> Seq.concat
        |> Seq.choose (fun obj ->
            try Some(LibraryEntry.Construct(obj))
            with | _ ->
                let objName = if String.IsNullOrWhiteSpace(obj.Name) then "<unnamed-object>" else obj.Name
                notify.Invoke(sprintf "Warning: Failed to load material library object '%s'. Check the definition in the IDF.\n" objName)
                None)
    SortedSet(entries) :> ISet<LibraryEntry>