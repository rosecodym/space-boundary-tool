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
            internal double equalityTolerance;
            internal double maxPairDistance;
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

        [DllImport("SBT-IFC.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl, EntryPoint = "load_and_run_from")]
        private static extern int LoadAndRunFrom(
            string inputFilename,
            string outputFilename,
            SBCalculationOptions opts,
            out IntPtr elements,
            out uint elementCount,
            out IntPtr spaces,
            out uint spaceCount,
            out IntPtr spaceBoundaries,
            out uint sbCount);

        [DllImport("SBT-IFC.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl, EntryPoint = "free_elements")]
        private static extern void FreeElements(IntPtr elements, uint count);

        [DllImport("SBT-IFC.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl, EntryPoint = "free_spaces")]
        private static extern void FreeSpaces(IntPtr spaces, uint count);

        [DllImport("SBT-IFC.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl, EntryPoint = "free_space_boundaries")]
        private static extern void FreeSpaceBoundaries(IntPtr sbs, uint count);

        [Flags]
        public enum SbtFlags : int
        {
            None = 0x0,
            VerboseBlocks = 0x2,
            VerboseStacks = 0x4,
            VerboseGeometry = 0x8,
            SkipWallColumnCheck = 0x10,
            SkipSlabColumnCheck = 0x20,
            SkipWallSlabCheck = 0x40,
            VerboseFenestrations = 0x80,
            Skip3rdLevelCheck = 0x100,
            VerboseSpaces = 0x200,
            ExpensiveChecks = 0x400,
            VerboseLevels = 0x800,
            VerboseCombinatorics = 0x1000
        }

        public enum SbtResult : int
        {
            Ok = 0,
            Unknown = -1,
            AssertionFailed = -2
        }

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void MessageDelegate(string msg);

        public static void CalculateSpaceBoundaries(
            ICollection<CoreTypes.ElementInfo> elements,
            ICollection<CoreTypes.SpaceInfo> spaces,
            out ICollection<CoreTypes.SpaceBoundary> spaceBoundaries,
            SbtFlags flags = SbtFlags.None,
            double internalEpsilon = 0.01,
            double maxPairDistance = 3.0,
            int spaceVerificationTimeout = -1,
            MessageDelegate notifyMsg = null,
            MessageDelegate warningMsg = null,
            MessageDelegate errorMsg = null)
        {
            SBCalculationOptions opts;
            opts.flags = flags;
            opts.spaceVerificationTimeout = spaceVerificationTimeout;
            opts.equalityTolerance = internalEpsilon;
            opts.maxPairDistance = maxPairDistance;
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
                    else if (res == SbtResult.AssertionFailed)
                    {
                        throw new Exceptions.SbtAssertionFailedException();
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
            out ICollection<CoreTypes.SpaceInfo> spaces,
            out ICollection<CoreTypes.SpaceBoundary> spaceBoundaries,
            SbtFlags flags = SbtFlags.None,
            double internalEpsilon = 0.01,
            double maxPairDistance = 3.0,
            IEnumerable<string> spaceFilter = null,
            IEnumerable<string> elementFilter = null,
            int spaceVerificationTimeout = -1,
            MessageDelegate notifyMsg = null,
            MessageDelegate warningMsg = null,
            MessageDelegate errorMsg = null)
        {
            SBCalculationOptions opts;
            opts.flags = flags;
            opts.spaceVerificationTimeout = spaceVerificationTimeout;
            opts.equalityTolerance = internalEpsilon;
            opts.maxPairDistance = maxPairDistance;
            opts.notifyFunc = notifyMsg != null ? Marshal.GetFunctionPointerForDelegate(notifyMsg) : IntPtr.Zero;
            opts.warnFunc = warningMsg != null ? Marshal.GetFunctionPointerForDelegate(warningMsg) : IntPtr.Zero;
            opts.errorFunc = errorMsg != null ? Marshal.GetFunctionPointerForDelegate(errorMsg) : IntPtr.Zero;

            var actualSpaceFilter = new List<string>(spaceFilter.Where(guid => !String.IsNullOrWhiteSpace(guid)));
            var actualElementFilter = new List<string>(elementFilter.Where(guid => !String.IsNullOrWhiteSpace(guid)));

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

            int res = LoadAndRunFrom(inputFilename, outputFilename, opts, out nativeElements, out elementCount, out nativeSpaces, out spaceCount, out nativeSbs, out spaceBoundaryCount);

            if (res != 0)
            {
                throw new Exception("Processing from the IFC file failed.");
            }

            elements = new List<CoreTypes.ElementInfo>();
            for (uint i = 0; i < elementCount; ++i)
            {
                IntPtr e = Marshal.ReadIntPtr(nativeElements, Marshal.SizeOf(typeof(IntPtr)) * (int)i);
                NativeCoreTypes.ElementInfo native = (NativeCoreTypes.ElementInfo)Marshal.PtrToStructure(e, typeof(NativeCoreTypes.ElementInfo));
                elements.Add(new CoreTypes.ElementInfo(native));
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
