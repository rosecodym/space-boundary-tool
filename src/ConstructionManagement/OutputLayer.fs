namespace ConstructionManagement

open System

open IdfToolbox.Base
open IdfToolbox.Idf

open MaterialLibrary

[<AbstractClass>]
type OutputLayer () =
    abstract Name : string with get
    abstract IsAirLayer : bool with get
    abstract IsIRTransparent : bool with get

    abstract member AddToIdfV710 : Idf -> unit
    abstract member AddToIdfV720 : Idf -> unit
    abstract member AddToIdfV800 : Idf -> unit

    override this.Equals(obj:Object) = 
        match obj with
        | :? OutputLayer as layer -> this.Equals(layer)
        | _ -> false

    override this.GetHashCode() = this.Name.GetHashCode()

    interface IEquatable<OutputLayer> with
        member this.Equals(other) =
            if (Object.ReferenceEquals(other, null)) then false
            else if (Object.ReferenceEquals(this, other)) then true
            else if (this.GetType() <> other.GetType()) then false
            else this.Name = other.Name

    interface IComparable<OutputLayer> with
        member this.CompareTo(other) = this.Name.CompareTo(other.Name)

    interface IComparable with
        member this.CompareTo(obj) =
            match obj with
            | null -> 1
            | :? OutputLayer as layer -> (this :> IComparable<OutputLayer>).CompareTo(layer)
            | _ -> raise (ArgumentException())

type internal OutputLayerAirGap (resistance:float) =
    inherit OutputLayer()
    override this.Name = sprintf "Air gap with resistance %f" resistance
    override this.IsAirLayer = true
    override this.IsIRTransparent = false

    override this.AddToIdfV710(idf) = this.AddToIdf(idf)
    override this.AddToIdfV720(idf) = this.AddToIdf(idf)
    override this.AddToIdfV800(idf) = this.AddToIdf(idf)

    member private this.AddToIdf(idf) =
        let obj = idf.CreateObject("Material:AirGap")
        obj.Fields.["Name"].Value <- Alpha(this.Name)
        obj.Fields.["Thermal Resistance"].Value <- Real(resistance)

type internal OutputLayerGas (name, entry:LibraryEntryGas, thickness) =
    inherit OutputLayer()
    override this.Name = name
    override this.IsAirLayer = false
    override this.IsIRTransparent = false

    override this.AddToIdfV710(idf) = this.CreateWithCommonProps idf |> ignore
    override this.AddToIdfV720(idf) = 
        this.CreateWithCommonProps idf |> this.AddPost710Props
    override this.AddToIdfV800(idf) = 
        this.CreateWithCommonProps idf |> this.AddPost710Props

    member private this.CreateWithCommonProps idf =
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
        obj

    member private this.AddPost710Props obj = 
        obj.Fields.["Conductivity Coefficient C"].Value <-
            entry.ConductivityCoefficientC
        obj.Fields.["Viscosity Coefficient C"].Value <-
            entry.ViscosityCoefficientC
        obj.Fields.["Specific Heat Coefficient C"].Value <-
            entry.SpecificHeatCoefficientC

type internal OutputLayerGlazing (name, entry:LibraryEntryGlazing, thickness) =
    inherit OutputLayer()
    override this.Name = name
    override this.IsAirLayer = false
    override this.IsIRTransparent = false

    override this.AddToIdfV710(idf) = this.AddToIdf(idf)
    override this.AddToIdfV720(idf) = this.AddToIdf(idf)
    override this.AddToIdfV800(idf) = this.AddToIdf(idf)

    member private this.AddToIdf(idf) =
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

type internal OutputLayerInfraredTransparent () =
    inherit OutputLayer()
    override this.Name = "Infrared transparent material"
    override this.IsAirLayer = false
    override this.IsIRTransparent = true

    override this.AddToIdfV710(idf) = this.AddToIdf(idf)
    override this.AddToIdfV720(idf) = this.AddToIdf(idf)
    override this.AddToIdfV800(idf) = this.AddToIdf(idf)

    member private this.AddToIdf(idf) =
        let obj = idf.CreateObject("Material:InfraredTransparent")
        obj.Fields.["Name"].Value <- Alpha(this.Name)

type internal OutputLayerNoMass (name, libraryEntry:LibraryEntryNoMass) =
    inherit OutputLayer()
    override this.Name = name
    override this.IsAirLayer = false
    override this.IsIRTransparent = false

    override this.AddToIdfV710(idf) = this.AddToIdf(idf)
    override this.AddToIdfV720(idf) = this.AddToIdf(idf)
    override this.AddToIdfV800(idf) = this.AddToIdf(idf)

    member private this.AddToIdf(idf) =
        let obj = idf.CreateObject("Material:NoMass")
        obj.Fields.["Name"].Value <- Alpha(this.Name)
        obj.Fields.["Roughness"].Value <- libraryEntry.Roughness
        obj.Fields.["Thermal Resistance"].Value <- libraryEntry.ThermalResistance
        obj.Fields.["Thermal Absorptance"].Value <- libraryEntry.ThermalAbsorptance
        obj.Fields.["Solar Absorptance"].Value <- libraryEntry.SolarAbsorptance
        obj.Fields.["Visible Absorptance"].Value <- libraryEntry.VisibleAbsorptance

type internal OutputLayerOpaque (name, libraryEntry:LibraryEntryOpaque, thickness) =
    inherit OutputLayer()
    override this.Name = name
    override this.IsAirLayer = false
    override this.IsIRTransparent = false

    override this.AddToIdfV710(idf) = this.AddToIdf(idf)
    override this.AddToIdfV720(idf) = this.AddToIdf(idf)
    override this.AddToIdfV800(idf) = this.AddToIdf(idf)

    member private this.AddToIdf(idf) =
        let obj = idf.CreateObject("Material")
        obj.Fields.["Name"].Value <- Alpha(this.Name)
        obj.Fields.["Thickness"].Value <- Real(thickness)
        obj.Fields.["Roughness"].Value <- libraryEntry.Roughness
        obj.Fields.["Conductivity"].Value <- libraryEntry.Conductivity
        obj.Fields.["Density"].Value <- libraryEntry.Density
        obj.Fields.["Specific Heat"].Value <- libraryEntry.SpecificHeat
        obj.Fields.["Thermal Absorptance"].Value <- libraryEntry.ThermalAbsorptance
        obj.Fields.["Solar Absorptance"].Value <- libraryEntry.SolarAbsorptance
        obj.Fields.["Visible Absorptance"].Value <- libraryEntry.VisibleAbsorptance