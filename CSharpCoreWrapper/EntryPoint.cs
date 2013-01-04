using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;

namespace Sbt
{

    public static class EntryPoint
    {

        [StructLayout(LayoutKind.Sequential)]
        private struct SBCalculationOptions
        {
            internal SbtFlags flags;
            internal double lengthUnitsPerMeter;
            internal double maxPairDistanceInMeters;
            internal int spaceVerificationTimeout;
            internal IntPtr spaceFilter;
            internal uint spaceFilterCount;
            internal IntPtr elementFilter;
            internal uint elementFilterCount;
            internal IntPtr notifyFunc;
            internal IntPtr warnFunc;
            internal IntPtr errorFunc;
        }

        [DllImport("SBT-Core.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl, EntryPoint = "calculate_space_boundaries")]
        private static extern SbtResult CalculateSpaceBoundaries(
            uint elementCount,
            IntPtr elements,
            uint spaceCount,
            IntPtr spaces,
            out uint spaceBoundaryCount,
            out IntPtr spaceBoundaries,
            SBCalculationOptions opts);

        [DllImport("SBT-IFC.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl, EntryPoint = "execute")]
        private static extern IfcAdapterResult LoadAndRunFrom(
            string inputFilename,
            string outputFilename,
            SBCalculationOptions opts,
            out uint elementCount,
            out IntPtr elements,
            out IntPtr compositeLayerDXs,
            out IntPtr compositeLayerDYs,
            out IntPtr compositeLayerDZs,
            out uint spaceCount,
            out IntPtr spaces,
            out uint sbCount,
            out IntPtr spaceBoundaries);

        [DllImport("SBT-IFC.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl, EntryPoint = "release_elements")]
        private static extern void FreeElements(IntPtr elements, uint count);

        [DllImport("SBT-IFC.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl, EntryPoint = "release_spaces")]
        private static extern void FreeSpaces(IntPtr spaces, uint count);

        [DllImport("SBT-Core.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl, EntryPoint = "release_space_boundaries")]
        private static extern void FreeSpaceBoundaries(IntPtr sbs, uint count);

        [Flags]
        public enum SbtFlags : int
        {
            None = 0x0
        }

        public enum SbtResult : int
        {
            Ok = 0,
            TooComplicated = 1,
            Unknown = -1
        }

        public enum IfcAdapterResult : int
        {
            Ok = 0,
            EdmError = 1,
            TooComplicated = 2,
            Unknown = -1
        }

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void MessageDelegate(string msg);

        public static void CalculateSpaceBoundaries(
            ICollection<CoreTypes.ElementInfo> elements,
            ICollection<CoreTypes.SpaceInfo> spaces,
            out ICollection<CoreTypes.SpaceBoundary> spaceBoundaries,
            SbtFlags flags = SbtFlags.None,
            double lengthUnitsPerMeter = 1.0,
            double maxPairDistanceInMeters = 0.5,
            MessageDelegate notifyMsg = null,
            MessageDelegate warningMsg = null,
            MessageDelegate errorMsg = null)
        {
            SBCalculationOptions opts;
            opts.flags = flags;
            opts.spaceVerificationTimeout = 0; // no longer does anything
            opts.lengthUnitsPerMeter = lengthUnitsPerMeter;
            opts.maxPairDistanceInMeters = maxPairDistanceInMeters;
            opts.notifyFunc = notifyMsg != null ? Marshal.GetFunctionPointerForDelegate(notifyMsg) : IntPtr.Zero;
            opts.warnFunc = warningMsg != null ? Marshal.GetFunctionPointerForDelegate(warningMsg) : IntPtr.Zero;
            opts.errorFunc = errorMsg != null ? Marshal.GetFunctionPointerForDelegate(errorMsg) : IntPtr.Zero;
            opts.elementFilter = IntPtr.Zero;
            opts.elementFilterCount = 0;
            opts.spaceFilter = IntPtr.Zero;
            opts.spaceFilterCount = 0;

            uint elementCount = (uint)elements.Count;
            uint spaceCount = (uint)spaces.Count;
            uint spaceBoundaryCount;

            spaceBoundaries = new List<CoreTypes.SpaceBoundary>();

            SbtResult res = SbtResult.Unknown;

            IntPtr nativeSbs;

            using (CoreTypes.ElementList es = new CoreTypes.ElementList(elements))
            {
                using (CoreTypes.SpaceList ss = new CoreTypes.SpaceList(spaces))
                {
                    res = CalculateSpaceBoundaries(elementCount, es.NativePtr, spaceCount, ss.NativePtr, out spaceBoundaryCount, out nativeSbs, opts);
                    if (res == SbtResult.Ok)
                    {
                        for (uint i = 0; i < spaceBoundaryCount; ++i)
                        {
                            IntPtr sb = Marshal.ReadIntPtr(nativeSbs, Marshal.SizeOf(typeof(IntPtr)) * (int)i);
                            NativeCoreTypes.SpaceBoundary native = (NativeCoreTypes.SpaceBoundary)Marshal.PtrToStructure(sb, typeof(NativeCoreTypes.SpaceBoundary));
                            spaceBoundaries.Add(new CoreTypes.SpaceBoundary(native));
                        }
                        CoreTypes.SpaceBoundary.LinkOpposites(spaceBoundaries);
                        CoreTypes.SpaceBoundary.LinkContaining(spaceBoundaries);
                        CoreTypes.SpaceBoundary.LinkSpaces(spaceBoundaries, spaces);
                        CoreTypes.SpaceBoundary.LinkElements(spaceBoundaries, elements);
                    }
                    else if (res == SbtResult.TooComplicated)
                    {
                        throw new Exceptions.TooComplicatedException();
                    }
                    else {
                        throw new Exceptions.SbtException();
                    }
                }
            }
        }

        public static void CalculateSpaceBoundariesFromIfc(
            string inputFilename,
            string outputFilename,
            out IList<CoreTypes.ElementInfo> elements,
            out IList<Tuple<double, double, double>> compositeDirs,
            out ICollection<CoreTypes.SpaceInfo> spaces,
            out ICollection<CoreTypes.SpaceBoundary> spaceBoundaries,
            SbtFlags flags = SbtFlags.None,
            double maxPairDistanceInMeters = 0.5,
            IEnumerable<string> spaceFilter = null,
            IEnumerable<string> elementFilter = null,
            MessageDelegate notifyMsg = null,
            MessageDelegate warningMsg = null,
            MessageDelegate errorMsg = null)
        {
            SBCalculationOptions opts;
            opts.flags = flags;
            opts.spaceVerificationTimeout = 0; // no longer does anything
            opts.lengthUnitsPerMeter = 1.0;
            opts.maxPairDistanceInMeters = maxPairDistanceInMeters;
            opts.notifyFunc = notifyMsg != null ? Marshal.GetFunctionPointerForDelegate(notifyMsg) : IntPtr.Zero;
            opts.warnFunc = warningMsg != null ? Marshal.GetFunctionPointerForDelegate(warningMsg) : IntPtr.Zero;
            opts.errorFunc = errorMsg != null ? Marshal.GetFunctionPointerForDelegate(errorMsg) : IntPtr.Zero;

            var actualSpaceFilter = spaceFilter == null ? new List<string>() : new List<string>(spaceFilter.Where(guid => !String.IsNullOrWhiteSpace(guid)));
            var actualElementFilter = elementFilter == null ? new List<string>() : new List<string>(elementFilter.Where(guid => !String.IsNullOrWhiteSpace(guid)));

            if (actualSpaceFilter == null)
            {
                opts.spaceFilter = IntPtr.Zero;
                opts.spaceFilterCount = 0;
            }
            else
            {
                opts.spaceFilterCount = (uint)actualSpaceFilter.Count;
                opts.spaceFilter = Marshal.AllocHGlobal(actualSpaceFilter.Count * Marshal.SizeOf(typeof(IntPtr)));
                for (int i = 0; i < actualSpaceFilter.Count; ++i)
                {
                    Marshal.WriteIntPtr(opts.spaceFilter, i * Marshal.SizeOf(typeof(IntPtr)), Marshal.StringToHGlobalAnsi(actualSpaceFilter[i]));
                }
            }

            if (actualElementFilter == null)
            {
                opts.elementFilter = IntPtr.Zero;
                opts.elementFilterCount = 0;
            }
            else
            {
                opts.elementFilterCount = (uint)actualElementFilter.Count;
                opts.elementFilter = Marshal.AllocHGlobal(actualElementFilter.Count * Marshal.SizeOf(typeof(IntPtr)));
                for (int i = 0; i < actualElementFilter.Count; ++i)
                {
                    Marshal.WriteIntPtr(opts.elementFilter, i * Marshal.SizeOf(typeof(IntPtr)), Marshal.StringToHGlobalAnsi(actualElementFilter[i]));
                }
            }

            uint elementCount;
            uint spaceCount;
            uint spaceBoundaryCount;

            IntPtr nativeElements;
            IntPtr nativeSpaces;
            IntPtr nativeSbs;

            IntPtr compositeDxsPtr;
            IntPtr compositeDysPtr;
            IntPtr compositeDzsPtr;

            IfcAdapterResult res = LoadAndRunFrom(
                inputFilename, 
                outputFilename,
                opts,
                out elementCount,
                out nativeElements,
                out compositeDxsPtr,
                out compositeDysPtr,
                out compositeDzsPtr,
                out spaceCount,
                out nativeSpaces,
                out spaceBoundaryCount, 
                out nativeSbs);

            if (res == IfcAdapterResult.EdmError)
            {
                throw new Exception("There was a problem reading or writing an IFC file.\n");
            }
            else if (res == IfcAdapterResult.TooComplicated)
            {
                throw new Exception("Geometry too complicated! Try simplifying the geometry around where the error occurred.\n");
            }
            else if (res != IfcAdapterResult.Ok)
            {
                throw new Exception("Unknown error during processing.\n");
            }

            elements = new List<CoreTypes.ElementInfo>();
            compositeDirs = new List<Tuple<double, double, double>>();
            var compositeDxs = new double[elementCount];
            var compositeDys = new double[elementCount];
            var compositeDzs = new double[elementCount];
            Marshal.Copy(compositeDxsPtr, compositeDxs, 0, (int)elementCount);
            Marshal.Copy(compositeDysPtr, compositeDys, 0, (int)elementCount);
            Marshal.Copy(compositeDzsPtr, compositeDzs, 0, (int)elementCount);
            for (uint i = 0; i < elementCount; ++i)
            {
                IntPtr e = Marshal.ReadIntPtr(nativeElements, Marshal.SizeOf(typeof(IntPtr)) * (int)i);
                NativeCoreTypes.ElementInfo native = (NativeCoreTypes.ElementInfo)Marshal.PtrToStructure(e, typeof(NativeCoreTypes.ElementInfo));
                elements.Add(new CoreTypes.ElementInfo(native));
                if (native.type == NativeCoreTypes.ElementType.Slab)
                {
                    compositeDirs.Add(Tuple.Create(0.0, 0.0, 1.0));
                }
                else
                {
                    var cdx = compositeDxs[i];
                    var cdy = compositeDys[i];
                    var cdz = compositeDzs[i];
                    compositeDirs.Add(Tuple.Create(cdx, cdy, cdz));
                }
            }

            spaces = new List<CoreTypes.SpaceInfo>();
            for (uint i = 0; i < spaceCount; ++i)
            {
                IntPtr s = Marshal.ReadIntPtr(nativeSpaces, Marshal.SizeOf(typeof(IntPtr)) * (int)i);
                NativeCoreTypes.SpaceInfo native = (NativeCoreTypes.SpaceInfo)Marshal.PtrToStructure(s, typeof(NativeCoreTypes.SpaceInfo));
                spaces.Add(new CoreTypes.SpaceInfo(native));
            }

            spaceBoundaries = new List<CoreTypes.SpaceBoundary>();
            for (uint i = 0; i < spaceBoundaryCount; ++i)
            {
                IntPtr sb = Marshal.ReadIntPtr(nativeSbs, Marshal.SizeOf(typeof(IntPtr)) * (int)i);
                NativeCoreTypes.SpaceBoundary native = (NativeCoreTypes.SpaceBoundary)Marshal.PtrToStructure(sb, typeof(NativeCoreTypes.SpaceBoundary));
                spaceBoundaries.Add(new CoreTypes.SpaceBoundary(native));
            }

            CoreTypes.SpaceBoundary.LinkOpposites(spaceBoundaries);
            CoreTypes.SpaceBoundary.LinkContaining(spaceBoundaries);
            CoreTypes.SpaceBoundary.LinkSpaces(spaceBoundaries, spaces);
            CoreTypes.SpaceBoundary.LinkElements(spaceBoundaries, elements);

            FreeElements(nativeElements, elementCount);
            FreeSpaces(nativeSpaces, spaceCount);
            FreeSpaceBoundaries(nativeSbs, spaceBoundaryCount);
        }
    }
}
