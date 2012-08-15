using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace GUI.Materials.Output
{
    abstract class MaterialLayer : IEquatable<MaterialLayer>
    {
        public abstract string Name { get; }

        public abstract void AddToIdfV710(LibIdf.Idf.Idf idf);

        public override bool Equals(object obj)
        {
            return Equals(obj as MaterialLayer);
        }

        public bool Equals(MaterialLayer other)
        {
            if (Object.ReferenceEquals(other, null)) { return false; }
            if (Object.ReferenceEquals(this, other)) { return true; }
            if (this.GetType() != other.GetType()) { return false; }
            return this.Name == other.Name;
        }

        public static bool operator ==(MaterialLayer a, MaterialLayer b)
        {
            if (Object.ReferenceEquals(a, null))
            {
                if (Object.ReferenceEquals(b, null)) { return true; }
                else { return false; }
            }
            return a.Equals(b);
        }

        public static bool operator !=(MaterialLayer a, MaterialLayer b)
        {
            return !(a == b);
        }

        public override int GetHashCode()
        {
            return Name.GetHashCode();
        }

        public override string ToString()
        {
            return Name;
        }
    }
}
