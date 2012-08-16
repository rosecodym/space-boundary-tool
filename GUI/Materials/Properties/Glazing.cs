using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace GUI.Materials.Properties
{
    class Glazing
    {
        readonly double solarTransmittance;
        readonly double frontSideSolarReflectance;
        readonly double backSideSolarReflectance;
        readonly double visibleTransmittance;
        readonly double frontSideVisibleReflectance;
        readonly double backSideVisibleReflectance;
        readonly double infraredTransmittance;
        readonly double frontSideHemisphericalEmissivity;
        readonly double backSideHemisphericalEmissivity;
        readonly double conductivity;

        public double SolarTransmittanceAtNormalIncidence { get { return solarTransmittance; } }
        public double FrontSideSolarReflectanceAtNormalIncidence { get { return frontSideSolarReflectance; } }
        public double BackSideSolarReflectanceAtNormalIncidence { get { return backSideSolarReflectance; } }
        public double VisibleTransmittanceAtNormalIncidence { get { return visibleTransmittance; } }
        public double FrontSideVisibleReflectanceAtNormalIncidence { get { return frontSideVisibleReflectance; } }
        public double BackSideVisibleReflectanceAtNormalIncidence { get { return backSideVisibleReflectance; } }
        public double InfraredTransmittanceAtNormalIncidence { get { return infraredTransmittance; } }
        public double FrontSideInfraredHemisphericalEmissivity { get { return frontSideHemisphericalEmissivity; } }
        public double BackSideInfraredHemisphericalEmissivity { get { return backSideHemisphericalEmissivity; } }
        public double Conductivity { get { return conductivity; } }

        public Glazing(
            double solarTransmittanceAtNormalIncidence,
            double frontSideSolarReflectanceAtNormalIncidence,
            double backSideSolarReflectanceAtNormalIncidence,
            double visibleTransmittanceAtNormalIncidence,
            double frontSideVisibleReflectanceAtNormalIncidence,
            double backSideVisibleReflectanceAtNormalIncidence,
            double infraredTransmittanceAtNormalIncidence,
            double frontSideInfraredHemisphericalEmissivity,
            double backSideInfraredHemisphericalEmissivity,
            double conductivity)
        {
            solarTransmittance = solarTransmittanceAtNormalIncidence;
            frontSideSolarReflectance = frontSideSolarReflectanceAtNormalIncidence;
            backSideSolarReflectance = backSideSolarReflectanceAtNormalIncidence;
            visibleTransmittance = visibleTransmittanceAtNormalIncidence;
            frontSideVisibleReflectance = frontSideVisibleReflectanceAtNormalIncidence;
            backSideVisibleReflectance = backSideVisibleReflectanceAtNormalIncidence;
            infraredTransmittance = infraredTransmittanceAtNormalIncidence;
            frontSideHemisphericalEmissivity = frontSideInfraredHemisphericalEmissivity;
            backSideHemisphericalEmissivity = backSideInfraredHemisphericalEmissivity;
            this.conductivity = conductivity;
        }
    }
}
