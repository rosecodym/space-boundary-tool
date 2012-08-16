using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using LibIdf.Idf;

namespace GUI.Materials.Output
{
    class MaterialLayerGlazing : MaterialLayer
    {
        readonly private string baseName;
        readonly private Properties.Glazing props;
        readonly private double thickness;

        public MaterialLayerGlazing(string name, Properties.Glazing props, double thickness)
        {
            this.baseName = name;
            this.props = props;
            this.thickness = thickness;
        }

        public override string Name
        {
            get { return String.Format("{0} ({1})", baseName, thickness); }
        }

        public override void AddToIdfV710(Idf idf)
        {
            IdfObject obj = idf.CreateObject("WindowMaterial:Glazing");
            obj.Fields["Name"].Value = Name;
            obj.Fields["Optical Data Type"].Value = "SpectralAverage";
            obj.Fields["Thickness"].Value = thickness;
            obj.Fields["Solar Transmittance at Normal Incidence"].Value = props.SolarTransmittanceAtNormalIncidence;
            obj.Fields["Front Side Solar Reflectance at Normal Incidence"].Value = props.FrontSideSolarReflectanceAtNormalIncidence;
            obj.Fields["Back Side Solar Reflectance at Normal Incidence"].Value = props.BackSideSolarReflectanceAtNormalIncidence;
            obj.Fields["Visible Transmittance at Normal Incidence"].Value = props.VisibleTransmittanceAtNormalIncidence;
            obj.Fields["Front Side Visible Reflectance at Normal Incidence"].Value = props.FrontSideVisibleReflectanceAtNormalIncidence;
            obj.Fields["Back Side Visible Reflectance at Normal Incidence"].Value = props.BackSideVisibleReflectanceAtNormalIncidence;
            obj.Fields["Infrared Transmittance at Normal Incidence"].Value = props.InfraredTransmittanceAtNormalIncidence;
            obj.Fields["Front Side Infrared Hemispherical Emissivity"].Value = props.FrontSideInfraredHemisphericalEmissivity;
            obj.Fields["Back Side Infrared Hemispherical Emissivity"].Value = props.BackSideInfraredHemisphericalEmissivity;
            obj.Fields["Conductivity"].Value = props.Conductivity;
        }
    }
}
