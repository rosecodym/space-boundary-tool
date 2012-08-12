using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace GUI.Constructions
{
    class MaterialLayerOpaque : MaterialLayer
    {
        readonly string name;
        readonly MaterialRoughness roughness;
        readonly double conductivity;
        readonly double density;
        readonly double specificHeat;
        readonly double thermalAbsorptance;
        readonly double solarAbsorptance;
        readonly double visibleAbsorptance;

        public override string Name { get { return name; } }
        public MaterialRoughness Roughness { get { return roughness; } }
        public double Conductivity { get { return conductivity; } }
        public double Density { get { return density; } }
        public double SpecificHeat { get { return specificHeat; } }
        public double ThermalAbsorptance { get { return thermalAbsorptance; } }
        public double SolarAbsorptance { get { return solarAbsorptance; } }
        public double VisibleAbsorptance { get { return visibleAbsorptance; } }

        public MaterialLayerOpaque(
            string name,
            MaterialRoughness roughness,
            double conductivity,
            double density,
            double specificHeat,
            double thermalAbsorptance,
            double solarAbsorptance,
            double visibleAbsorptance)
        {
            this.name = name;
            this.roughness = roughness;
            this.conductivity = conductivity;
            this.density = density;
            this.specificHeat = specificHeat;
            this.thermalAbsorptance = thermalAbsorptance;
            this.solarAbsorptance = solarAbsorptance;
            this.visibleAbsorptance = visibleAbsorptance;
        }

        public override void AddToIdfV710(LibIdf.Idf.Idf idf)
        {
            LibIdf.Idf.IdfObject obj = idf.CreateObject("Material:NoMass");
            obj.Fields["Name"].Value = Name;
            obj.Fields["Roughness"].Value = roughness.ToString();
            obj.Fields["Conductivity"].Value = conductivity;
            obj.Fields["Density"].Value = density;
            obj.Fields["Specific Heat"].Value = specificHeat;
            obj.Fields["Thermal Absorptance"].Value = thermalAbsorptance;
            obj.Fields["Solar Absorptance"].Value = solarAbsorptance;
            obj.Fields["Visible Absorptance"].Value = visibleAbsorptance;
        }
    }
}
