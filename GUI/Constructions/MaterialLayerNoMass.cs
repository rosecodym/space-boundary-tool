using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace GUI.Constructions
{
    class MaterialLayerNoMass : MaterialLayer
    {
        private readonly string name;
        private readonly MaterialRoughness roughness;
        private readonly double resistance;

        public MaterialLayerNoMass(string name, MaterialRoughness roughness, double resistance)
        {
            this.name = name;
            this.roughness = roughness;
            this.resistance = resistance;
        }

        public override string Name
        {
            get { return name; }
        }

        public override void AddToIdfV710(LibIdf.Idf.Idf idf)
        {
            LibIdf.Idf.IdfObject obj = idf.CreateObject("Material:NoMass");
            obj.Fields["Name"].Value = Name;
            obj.Fields["Roughness"].Value = roughness.ToString();
            obj.Fields["Thermal Resistance"].Value = resistance;
        }
    }
}
