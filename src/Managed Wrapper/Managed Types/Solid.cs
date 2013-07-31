using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Sbt.CoreTypes
{
    public abstract class Solid
    {
        public abstract IList<Face> ToFaces();

        internal abstract Solid ScaledBy(double factor);
        internal abstract NativeCoreTypes.Solid ToNative();

        internal static Solid FromNative(NativeCoreTypes.Solid s)
        {
            if (s.repType == NativeCoreTypes.SolidRepType.ExtrudedAreaSolid)
            {
                return new ExtrudedAreaSolid(s.asExt);
            }
            else { return new Brep(s.asBrep); }
        }
    }
}
