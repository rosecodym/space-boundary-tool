using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace GUI.Constructions
{
    abstract class MaterialLayer : IEquatable<MaterialLayer>
    {
        public abstract string Name { get; }

        public abstract void AddToIdfV710(LibIdf.Idf.Idf idf);

        public bool Equals(MaterialLayer other)
        {
            return other != null && Name == other.Name;
        }

        public override bool Equals(object obj)
        {
            return Equals(obj as MaterialLayer);
        }

        public static bool operator ==(MaterialLayer a, MaterialLayer b)
        {
            return a.Name == b.Name;
        }

        public static bool operator !=(MaterialLayer a, MaterialLayer b)
        {
            return !(a == b);
        }

        public override int GetHashCode()
        {
            return Name.GetHashCode();
        }
    }
}
