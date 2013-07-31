using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;

using NativeFace = Sbt.NativeCoreTypes.Face;

namespace Sbt.CoreTypes
{
    public class Brep : Solid
    {
        private readonly List<Face> faces;
        public IList<Face> Faces { get { return faces.AsReadOnly(); } }
        public override IList<Face> ToFaces() { return Faces; }

        internal Brep(NativeCoreTypes.Brep fromNative)
        {
            faces = new List<Face>();
            var firstNativeFace = fromNative.faces;
            var nativeFaceSize = Marshal.SizeOf(typeof(NativeFace));
            for (int i = 0; i < fromNative.faceCount; ++i)
            {
                var ptr = firstNativeFace + i * nativeFaceSize;
                var faceObj = Marshal.PtrToStructure(ptr, typeof(NativeFace));
                faces.Add(new Face((NativeFace)faceObj));
            }
        }

        public Brep(IEnumerable<Face> fcs) { faces = new List<Face>(fcs); }

        internal override Solid ScaledBy(double factor)
        {
            return new Brep(faces.Select(f => f.ScaledBy(factor)));
        }

        internal override NativeCoreTypes.Solid ToNative()
        {
            var native = new NativeCoreTypes.Solid(); // because of the union?
            native.repType = NativeCoreTypes.SolidRepType.Brep;
            native.asBrep.faceCount = (uint)faces.Count;
            var faceSize = Marshal.SizeOf(typeof(NativeFace));
            native.asBrep.faces = Marshal.AllocHGlobal(faces.Count * faceSize);
            for (int i = 0; i < faces.Count; ++i)
            {
                var natStruct = faces[i].ToNative();
                var loc = native.asBrep.faces + faceSize * i;
                Marshal.StructureToPtr(natStruct, loc, false);
            }
            return native;
        }
    }
}
