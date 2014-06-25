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
            internal double tolernaceInMeters;
            internal int unused;
            internal IntPtr spaceFilter;
            internal uint spaceFilterCount;
            internal IntPtr elementFilter;
            internal uint elementFilterCount;
            internal IntPtr notifyFunc;
            internal IntPtr warnFunc;
            internal IntPtr errorFunc;
        }

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
            out IntPtr spaceBoundaries,
            out IntPtr correctedAreas,
            out int totalPoints,
            out int totalEdges,
            out int totalFaces,
            out int totalSolids);

        [DllImport("SBT-IFC.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl, EntryPoint = "release_elements")]
        private static extern void FreeElements(IntPtr elements, uint count);

        [DllImport("SBT-IFC.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl, EntryPoint = "release_spaces")]
        private static extern void FreeSpaces(IntPtr spaces, uint count);

        [DllImport("SBT-Core.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl, EntryPoint = "release_space_boundaries")]
        private static extern void FreeSpaceBoundaries(IntPtr sbs, uint count);

        [DllImport("SBT-IFC.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl, EntryPoint = "release_corrected_areas")]
        private static extern void FreeCorrectedAreas(IntPtr areas);

        [Flags]
        public enum SbtFlags : int
        {
            None = 0x0
        }

        public enum IfcAdapterResult : int
        {
            Ok = 0,
            IfcError = 1,
            StackOverflow = 2,
            InvalidArguments = 3,
            Unknown = -1
        }

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void MessageDelegate(string msg);

        public static void CalculateSpaceBoundariesFromIfc(
            string inputFilename,
            string outputFilename,
            out IList<CoreTypes.ElementInfo> elements,
            out IList<Tuple<double, double, double>> compositeDirs,
            out ICollection<CoreTypes.SpaceInfo> spaces,
            out IList<CoreTypes.SpaceBoundary> spaceBoundaries,
            out float[] correctedSBAreas,
            out int pointCount,
            out int edgeCount,
            out int faceCount,
            out int solidCount,
            double tolernace = 0.01,
            SbtFlags flags = SbtFlags.None,
            double maxPairDistanceInMeters = 0.5,
            IEnumerable<string> spaceFilter = null,
            IEnumerable<string> elementFilter = null,
            MessageDelegate notifyMsg = null,
            MessageDelegate warningMsg = null,
            MessageDelegate errorMsg = null)
        {
            if (inputFilename == null)
            {
                throw new ArgumentException(
                    "An input file must be specified",
                    "inputFilename");
            }
            SBCalculationOptions opts;
            opts.flags = flags;
            opts.lengthUnitsPerMeter = 1.0;
            opts.maxPairDistanceInMeters = maxPairDistanceInMeters;
            opts.tolernaceInMeters = tolernace;
            opts.unused = 0;
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

            uint eCount;
            uint spaceCount;
            uint spaceBoundaryCount;

            IntPtr nativeElements;
            IntPtr nativeSpaces;
            IntPtr nativeSbs;

            IntPtr compositeDxsPtr;
            IntPtr compositeDysPtr;
            IntPtr compositeDzsPtr;

            IntPtr correctedAreas;

            IfcAdapterResult res = LoadAndRunFrom(
                inputFilename, 
                outputFilename,
                opts,
                out eCount,
                out nativeElements,
                out compositeDxsPtr,
                out compositeDysPtr,
                out compositeDzsPtr,
                out spaceCount,
                out nativeSpaces,
                out spaceBoundaryCount, 
                out nativeSbs,
                out correctedAreas,
                out pointCount,
                out edgeCount,
                out faceCount,
                out solidCount);

            if (res == IfcAdapterResult.IfcError)
            {
                throw new Exception("There was a problem reading or writing an IFC file.\n");
            }
            else if (res == IfcAdapterResult.StackOverflow)
            {
                throw new Exception("Stack overflow. Please report this SBT bug.\n");
            }
            else if (res == IfcAdapterResult.InvalidArguments)
            {
                throw new Exception("Invalid arguments to the IFC adapter. Please report this SBT bug.\n");
            }
            else if (res != IfcAdapterResult.Ok)
            {
                throw new Exception("Unknown error during processing.\n");
            }

            elements = new List<CoreTypes.ElementInfo>();
            compositeDirs = new List<Tuple<double, double, double>>();
            if (eCount > 0)
            {
                var compositeDxs = new double[eCount];
                var compositeDys = new double[eCount];
                var compositeDzs = new double[eCount];
                Marshal.Copy(compositeDxsPtr, compositeDxs, 0, (int)eCount);
                Marshal.Copy(compositeDysPtr, compositeDys, 0, (int)eCount);
                Marshal.Copy(compositeDzsPtr, compositeDzs, 0, (int)eCount);
                for (uint i = 0; i < eCount; ++i)
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
            }

            spaces = new List<CoreTypes.SpaceInfo>();
            for (uint i = 0; i < spaceCount; ++i)
            {
                IntPtr s = Marshal.ReadIntPtr(nativeSpaces, Marshal.SizeOf(typeof(IntPtr)) * (int)i);
                NativeCoreTypes.SpaceInfo native = (NativeCoreTypes.SpaceInfo)Marshal.PtrToStructure(s, typeof(NativeCoreTypes.SpaceInfo));
                spaces.Add(new CoreTypes.SpaceInfo(native));
            }
            if (spaces.Count == 0)
            {
                System.Windows.MessageBox.Show("Your IFC file does not contain any spaces (IfcSpaces). Spaces are required to calculate space boundaries.");
            }

            int sbCount = (int)spaceBoundaryCount;

            var ptrSize = Marshal.SizeOf(typeof(IntPtr));
            spaceBoundaries = new List<CoreTypes.SpaceBoundary>();
            for (int i = 0; i < sbCount; ++i)
            {
                var sb = Marshal.ReadIntPtr(nativeSbs, ptrSize * i);
                NativeCoreTypes.SpaceBoundary native = (NativeCoreTypes.SpaceBoundary)Marshal.PtrToStructure(sb, typeof(NativeCoreTypes.SpaceBoundary));
                spaceBoundaries.Add(new CoreTypes.SpaceBoundary(native));
            }

            correctedSBAreas = new float[sbCount];
            Marshal.Copy(correctedAreas, correctedSBAreas, 0, sbCount);

            System.Diagnostics.Debug.Assert(
                !spaceBoundaries.Any(_ => true) ||
                spaceBoundaries
                .SelectMany(sb => sb.MaterialLayers)
                .Select(layer => layer.Id).Max() - 1 <
                compositeDirs.Count);

            CoreTypes.SpaceBoundary.LinkOpposites(spaceBoundaries);
            CoreTypes.SpaceBoundary.LinkContaining(spaceBoundaries);
            CoreTypes.SpaceBoundary.LinkSpaces(spaceBoundaries, spaces);
            CoreTypes.SpaceBoundary.LinkElements(spaceBoundaries, elements);

            FreeElements(nativeElements, eCount);
            FreeSpaces(nativeSpaces, spaceCount);
            FreeSpaceBoundaries(nativeSbs, spaceBoundaryCount);
            FreeCorrectedAreas(correctedAreas);
        }
    }
}
