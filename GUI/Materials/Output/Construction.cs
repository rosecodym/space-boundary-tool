using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace GUI.Materials.Output
{
    class Construction : IEquatable<Construction>
    {
        readonly private IList<string> layerNames;

        public Construction(IList<MaterialLayer> layers)
        {
            layerNames = new List<string>(layers.Select(layer => layer.Name));
        }

        public string Name
        {
            get
            {
                if (layerNames.Count == 1) { return layerNames[0]; }
                else { return String.Format("Unnamed composite (id {0})", layerNames.GetHashCode()); }
            }
        }

        public IList<string> LayerNames { get { return layerNames; } }

        public override bool Equals(object obj)
        {
            return Equals(obj as Construction);
        }

        public bool Equals(Construction other)
        {
            if (Object.ReferenceEquals(other, null)) { return false; }
            if (Object.ReferenceEquals(this, other)) { return true; }
            return this.layerNames == other.layerNames;
        }

        public static bool operator ==(Construction a, Construction b)
        {
            if (Object.ReferenceEquals(a, null))
            {
                return Object.ReferenceEquals(b, null);
            }
            return a.Equals(b);
        }

        public static bool operator !=(Construction a, Construction b)
        {
            return !(a == b);
        }

        public override int GetHashCode()
        {
            return layerNames.GetHashCode();
        }
    }
}
