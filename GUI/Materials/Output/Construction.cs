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
                else { return String.Format("Unnamed composite (id {0})", this.GetHashCode()); }
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
            if (this.LayerNames.Count != other.LayerNames.Count) { return false; }
            for (int i = 0; i < layerNames.Count; ++i)
            {
                if (this.LayerNames[i] != other.LayerNames[i]) { return false; }
            }
            return true;
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
            // i use '!' as a separator because it can't legally appear in E+ construction names
            return String.Join("!", LayerNames).GetHashCode();
        }
    }
}
