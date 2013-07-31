using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;

namespace Sbt.NativeCoreTypes
{
    enum SolidRepType : int 
    { 
        Nothing, 
        Brep, 
        ExtrudedAreaSolid 
    }

    enum ElementType : int
    {
        Wall,
        Slab,
        Door,
        Window,
        Column,
        Beam,
        Unknown
    }

    [StructLayout(LayoutKind.Sequential)]
    internal struct Point
    {
        internal double x;
        internal double y;
        internal double z;
    }

    [StructLayout(LayoutKind.Sequential)]
    internal struct Polyloop
    {
        internal uint vertexCount;
        internal IntPtr vertices;
        internal void Release()
        {
            Marshal.FreeHGlobal(vertices);
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    internal struct Face
    {
        internal Polyloop outerBoundary;
        internal uint voidCount;
        internal IntPtr voids;
        internal void Release()
        {
            outerBoundary.Release();
            if (voidCount > 0)
            {
                for (int i = 0; i < (uint)voidCount; ++i)
                {
                    ((Polyloop)Marshal.PtrToStructure(voids + i * Marshal.SizeOf(typeof(Polyloop)), typeof(Polyloop))).Release();
                }
                Marshal.FreeHGlobal(voids);
            }
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    internal struct Brep
    {
        internal uint faceCount;
        internal IntPtr faces;
        internal void Release()
        {
            for (int i = 0; i < (uint)faceCount; ++i)
            {
                ((Face)Marshal.PtrToStructure(faces + i * Marshal.SizeOf(typeof(Face)), typeof(Face))).Release();
            }
            Marshal.FreeHGlobal(faces);
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    internal struct ExtrudedAreaSolid
    {
        internal double dx;
        internal double dy;
        internal double dz;
        internal double depth;
        internal Face area;
        internal void Release()
        {
            area.Release();
        }
    }

    [StructLayout(LayoutKind.Explicit)]
    internal struct Solid
    {
        [FieldOffset(0)]
        internal SolidRepType repType;
        [FieldOffset(8)]
        internal Brep asBrep;
        [FieldOffset(8)]
        internal ExtrudedAreaSolid asExt;
        internal void Release()
        {
            if (repType == SolidRepType.Brep)
            {
                asBrep.Release();
            }
            else
            {
                asExt.Release();
            }
        }
    };

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    internal struct ElementInfo
    {
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
        internal string guid;
        internal ElementType type;
        internal int materialId;
        internal Solid geometry;
        internal IntPtr Alloc()
        {
            IntPtr native = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(ElementInfo)));
            Marshal.StructureToPtr(this, native, false);
            return native;
        }
        internal void Release()
        {
            geometry.Release();
        }
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    internal struct SpaceInfo
    {
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
        internal string guid;
        internal Solid geometry;
        internal IntPtr Alloc()
        {
            IntPtr native = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(SpaceInfo)));
            Marshal.StructureToPtr(this, native, false);
            return native;
        }
        internal void Release()
        {
            geometry.Release();
        }
    }

    [StructLayout(LayoutKind.Explicit, CharSet = CharSet.Ansi)]
    internal struct SpaceBoundary
    {
        [FieldOffset(0)]
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
        internal string guid;
        [FieldOffset(128)]
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
        internal string elementId;
        [FieldOffset(256)]
        internal Polyloop geometry;
        [FieldOffset(264)]
        internal double normalX;
        [FieldOffset(272)]
        internal double normalY;
        [FieldOffset(280)]
        internal double normalZ;
        [FieldOffset(288)]
        internal int isExternal;
        [FieldOffset(292)]
        internal int isVirtual;
        [FieldOffset(296)]
        internal IntPtr spaceInfo;
        [FieldOffset(300)]
        internal IntPtr opposite;
        [FieldOffset(304)]
        internal IntPtr parent;
        [FieldOffset(308)]
        internal uint materialLayerCount;
        [FieldOffset(312)]
        internal IntPtr layers;
        [FieldOffset(316)]
        internal IntPtr thicknesses;
    }
}
