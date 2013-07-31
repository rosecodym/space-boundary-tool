using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;

using NativeLoop = Sbt.NativeCoreTypes.Polyloop;

namespace Sbt.CoreTypes
{
    public class Face
    {
        private readonly Polyloop outerBoundary;
        private readonly List<Polyloop> voids;

        public Polyloop OuterBoundary { get { return outerBoundary; } }

        public IList<Polyloop> Voids { get { return voids.AsReadOnly(); } }

        public Face(Polyloop outerBoundary, IEnumerable<Polyloop> vs = null)
        {
            this.outerBoundary = outerBoundary;
            voids = vs != null ? new List<Polyloop>(vs) : new List<Polyloop>();
        }

        internal Face(NativeCoreTypes.Face fromNative)
        {
            this.outerBoundary = new Polyloop(fromNative.outerBoundary);
            voids = new List<Polyloop>();
            var nativeLoopSize = Marshal.SizeOf(typeof(NativeLoop));
            for (int i = 0; i < fromNative.voidCount; ++i)
            {
                var ptr = fromNative.voids + i * nativeLoopSize;
                var loopObj = Marshal.PtrToStructure(ptr, typeof(NativeLoop));
                voids.Add(new Polyloop((NativeLoop)loopObj));
            }
        }

        public Face Reversed()
        {
            var newVoids = voids.Select(v => v.Reversed());
            return new Face(OuterBoundary.Reversed(), newVoids);
        }

        internal Face ScaledBy(double factor)
        {
            var newVoids = voids.Select(v => v.ScaledBy(factor));
            return new Face(outerBoundary.ScaledBy(factor), newVoids);
        }

        internal Face Translated(double dx, double dy, double dz)
        {
            var newVs = voids.Select(v => v.Translated(dx, dy, dz));
            return new Face(this.OuterBoundary.Translated(dx, dy, dz), newVs);
        }

        internal NativeCoreTypes.Face ToNative()
        {
            NativeCoreTypes.Face native;
            native.outerBoundary = OuterBoundary.ToNative();
            native.voidCount = (uint)voids.Count;
            var natLoopSize = Marshal.SizeOf(typeof(NativeLoop));
            if (native.voidCount > 0)
            {
                native.voids = Marshal.AllocHGlobal(voids.Count * natLoopSize);
                for (int i = 0; i < voids.Count; ++i)
                {
                    var natStruct = voids[i].ToNative();
                    var loc = native.voids + natLoopSize * i;
                    Marshal.StructureToPtr(natStruct, loc, false);
                }
            }
            else { native.voids = IntPtr.Zero; }
            return native;
        }
    }
}
