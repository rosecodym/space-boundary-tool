using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;

using NativePoint = Sbt.NativeCoreTypes.Point;

namespace Sbt.CoreTypes
{
    public class Polyloop
    {
        private readonly List<Point> vertices;

        public IList<Point> Vertices { get { return vertices.AsReadOnly(); } }

        public Polyloop(IEnumerable<Point> points)
        {
            vertices = new List<Point>(points);
        }

        internal Polyloop(NativeCoreTypes.Polyloop fromNative)
        {
            vertices = new List<Point>();
            IntPtr firstNativePoint = fromNative.vertices;
            var nativePointSize = Marshal.SizeOf(typeof(NativePoint));
            for (int i = 0; i < (int)fromNative.vertexCount; ++i)
            {
                var ptr = firstNativePoint + i * nativePointSize;
                var ptObj = Marshal.PtrToStructure(ptr, typeof(NativePoint));
                vertices.Add(new Point((NativePoint)ptObj));
            }
        }

        public Polyloop Reversed() { return new Polyloop(Vertices.Reverse()); }

        internal Polyloop ScaledBy(double factor)
        {
            return new Polyloop(vertices.Select(p => p.ScaledBy(factor)));
        }

        internal Polyloop Translated(double dx, double dy, double dz)
        {
            return new Polyloop(vertices.Select(p => 
                new Point(p.X + dx, p.Y + dy, p.Z + dz)));
        }

        internal NativeCoreTypes.Polyloop ToNative()
        {
            NativeCoreTypes.Polyloop native;
            native.vertexCount = (uint)Vertices.Count;
            var natPtSize = Marshal.SizeOf(typeof(NativePoint));
            native.vertices = Marshal.AllocHGlobal(Vertices.Count * natPtSize);
            for (int i = 0; i < Vertices.Count; ++i)
            {
                var natStruct = vertices[i].ToNative();
                var loc = native.vertices + natPtSize * i;
                Marshal.StructureToPtr(natStruct, loc, false);
            }
            return native;
        }
    }
}
