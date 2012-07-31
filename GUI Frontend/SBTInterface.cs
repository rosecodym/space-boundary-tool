using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Linq;
using System.Text;

namespace GUI_Frontend
{
    static class SBTInterface
    {

        [DllImport("SBT-IFC.dll", SetLastError = true)]
        private static extern int add_to_ifc_file(string inputFileName, string outputFileName, SBCalculationOptions opts, out SBCounts counts);

        [Flags]
        enum SBTFlags
        {
            None = 0x000,
            VerboseBlocks = 0x002,
            VerboseStacks = 0x004,
            VerboseGeometry = 0x008,
            SkipWallColumnCheck = 0x010,
            SkipSlabColumnCheck = 0x020,
            SkipWallSlabCheck = 0x040,
            VerboseFenestrations = 0x080,
            Skip3rdLevelCheck = 0x100,
            VerboseSpaces = 0x200,
            ExpensiveChecks = 0x400
        }

        [StructLayout(LayoutKind.Sequential)]
        struct SBCalculationOptions
        {
            public SBTFlags flags;
            public double tolerance;
            public double maxPairDistance;
            public int spaceVerificationTimeout;
            public IntPtr spaceFilter;
            public UInt32 spaceFilterCount;
            public IntPtr elementFilter;
            public UInt32 elementFilterCount;
            public IntPtr notifyFunc;
            public IntPtr warnFunc;
            public IntPtr errorFunc;
        }

        [StructLayout(LayoutKind.Sequential)]
        internal struct SBCounts
        {
            public int boundedSpaceCount;
            IntPtr spaceGuids;
            IntPtr level2PhysicalInternal;
            IntPtr level2PhysicalExternal;
            IntPtr level3Internal;
            IntPtr level3External;
            IntPtr level4;
            IntPtr level5;
            IntPtr virt;
        }

        private static SBCalculationOptions BuildNativeOptionsStruct(SBTRunOptions opts)
        {
            SBCalculationOptions nativeOpts;
            nativeOpts.flags = SBTFlags.None;
            if (!opts.checkSlabColumn) { nativeOpts.flags |= SBTFlags.SkipSlabColumnCheck; }
            if (!opts.checkWallColumn) { nativeOpts.flags |= SBTFlags.SkipWallColumnCheck; }
            if (!opts.checkWallSlab) { nativeOpts.flags |= SBTFlags.SkipWallSlabCheck; }
            if (opts.expensiveChecks) { nativeOpts.flags |= SBTFlags.ExpensiveChecks; }
            if (opts.verboseBlocking) { nativeOpts.flags |= SBTFlags.VerboseBlocks; }
            if (opts.verboseStacking) { nativeOpts.flags |= SBTFlags.VerboseStacks; }
            if (opts.verboseGeometry) { nativeOpts.flags |= SBTFlags.VerboseGeometry; }
            if (opts.verboseFenestrations) { nativeOpts.flags |= SBTFlags.VerboseFenestrations; }
            if (opts.verboseSpaces) { nativeOpts.flags |= SBTFlags.VerboseSpaces; }
            nativeOpts.elementFilterCount = 0;
            nativeOpts.elementFilter = IntPtr.Zero;
            nativeOpts.spaceFilterCount = 0;
            nativeOpts.spaceFilter = IntPtr.Zero;
            nativeOpts.tolerance = 0.001;
            nativeOpts.maxPairDistance = 3.0; // this gets ignored by the ifc adapter anyway
            nativeOpts.spaceVerificationTimeout = opts.spaceVerificationTimeout;
            nativeOpts.notifyFunc = opts.notifyFunc;
            nativeOpts.warnFunc = opts.warnFunc;
            nativeOpts.errorFunc = opts.errorFunc;
            return nativeOpts;
        }

        internal static Nullable<SBCounts> InvokeSBT(SBTRunOptions opts, System.ComponentModel.BackgroundWorker worker) {
            SBCalculationOptions nativeOptions = BuildNativeOptionsStruct(opts);
            SBCounts counts;
            try
            {
                add_to_ifc_file(opts.inputFileName, opts.outputFileName, nativeOptions, out counts);
                return new Nullable<SBCounts>(counts);
            }
            catch (Exception e)
            {
                worker.ReportProgress(0, "Unhandled exception: " + e.Message);
                return null;
            }
        }

    }
}
