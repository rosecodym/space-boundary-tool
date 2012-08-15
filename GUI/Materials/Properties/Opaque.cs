using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using GUI.Materials.Enums;

namespace GUI.Materials.Properties
{
    class Opaque
    {
        readonly MaterialRoughness roughness;
        readonly double conductivity;
        readonly double density;
        readonly double specificHeat;
        readonly double thermalAbsorptance;
        readonly double solarAbsorptance;
        readonly double visibleAbsorptance;

        public MaterialRoughness Roughness { get { return roughness; } }
        public double Conductivity { get { return conductivity; } }
        public double Density { get { return density; } }
        public double SpecificHeat { get { return specificHeat; } }
        public double ThermalAbsorptance { get { return thermalAbsorptance; } }
        public double SolarAbsorptance { get { return solarAbsorptance; } }
        public double VisibleAbsorptance { get { return visibleAbsorptance; } }

        public Opaque(
            MaterialRoughness roughness,
            double conductivity,
            double density,
            double specificHeat,
            double thermalAbsorptance,
            double solarAbsorptance,
            double visibleAbsorptance)
        {
            this.roughness = roughness;
            this.conductivity = conductivity;
            this.density = density;
            this.specificHeat = specificHeat;
            this.thermalAbsorptance = thermalAbsorptance;
            this.solarAbsorptance = solarAbsorptance;
            this.visibleAbsorptance = visibleAbsorptance;
        }
    }
}
