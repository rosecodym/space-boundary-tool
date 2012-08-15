using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace GUI.Materials.LibraryEntries
{
    class Opaque
    {
        readonly private string name;
        readonly private Properties.Opaque props;

        public Opaque(LibIdf.Idf.IdfObject obj)
        {
            name = obj.Fields["Name"].Value;
            props = new Properties.Opaque(
                (Enums.MaterialRoughness)Enum.Parse(typeof(Enums.MaterialRoughness), obj.Fields["Roughness"].Value),
                obj.Fields["Conductivity"].Value,
                obj.Fields["Density"].Value,
                obj.Fields["Specific Heat"].Value,
                obj.Fields["Thermal Absorptance"].Value,
                obj.Fields["Solar Absorptance"].Value,
                obj.Fields["Visible Absorptance"].Value);
        }

        public string Name { get { return name; } }
        public Properties.Opaque Properties { get { return props; } }

        public override string ToString()
        {
            return Name;
        }
    }
}
