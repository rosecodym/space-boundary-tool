using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace GUI.Materials.Output
{
    class MaterialLayerOpaque : MaterialLayer
    {
        readonly private string baseName;
        readonly private Properties.Opaque props;
        readonly private double thickness;

        public MaterialLayerOpaque(string name, Properties.Opaque props, double thickness)
        {
            this.baseName = name;
            this.props = props;
            this.thickness = thickness;
        }

        public override string Name
        {
            get { return String.Format("{0} ({1})", baseName, thickness); }
        }

        public override void AddToIdfV710(LibIdf.Idf.Idf idf)
        {
            LibIdf.Idf.IdfObject obj = idf.CreateObject("Material");
            obj.Fields["Name"].Value = Name;
            obj.Fields["Thickness"].Value = thickness;
            obj.Fields["Roughness"].Value = props.Roughness.ToString();
            obj.Fields["Conductivity"].Value = props.Conductivity;
            obj.Fields["Density"].Value = props.Density;
            obj.Fields["Specific Heat"].Value = props.SpecificHeat;
            obj.Fields["Thermal Absorptance"].Value = props.ThermalAbsorptance;
            obj.Fields["Solar Absorptance"].Value = props.SolarAbsorptance;
            obj.Fields["Visible Absorptance"].Value = props.VisibleAbsorptance;
        }
    }
}
