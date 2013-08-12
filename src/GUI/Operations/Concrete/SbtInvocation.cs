using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Diagnostics;

using Normal = System.Tuple<double, double, double>;

namespace GUI.Operations
{
    class SbtInvocation : Operation<SbtInvocation.Parameters, SbtBuildingInformation>
    {
        [DllImport("Kernel32", EntryPoint = "GetCurrentThreadId")]
        public static extern Int32 GetCurrentWin32ThreadId();

        public class Parameters
        {
            public string InputFilename { get; set; }
            public string OutputFilename { get; set; }
            public Sbt.EntryPoint.SbtFlags Flags { get; set; }
            public IList<string> SpaceGuidFilter { get; set; }
            public IList<string> ElementGuidFilter { get; set; }
            public Sbt.EntryPoint.MessageDelegate NotifyMessage { get; set; }
            public Sbt.EntryPoint.MessageDelegate WarnMessage { get; set; }
            public Sbt.EntryPoint.MessageDelegate ErrorMessage { get; set; }
        }

        static private string GenerateTimeString(TimeSpan t)
        {
            string secondsComponent = t.Seconds == 1 ? "1 second" : String.Format("{0} seconds", t.Seconds);
            string minutesComponent =
                t.Minutes == 0 ? String.Empty :
                t.Minutes == 1 ? "1 minute, " : String.Format("{0} minutes, ", t.Minutes);
            string hoursComponent =
                t.Hours == 0 ? String.Empty :
                t.Hours == 1 ? "1 hour" : String.Format("{0} hours, ", t.Hours);
            return hoursComponent + minutesComponent + secondsComponent;
        }

        public SbtInvocation(ViewModel vm, Action completionAction)
            : base(_ => vm.UpdateGlobalStatus(), () => vm.ReasonForDisabledSBCalculation == null)
        {
            PrepareParameters = () =>
            {
                Parameters p = new Parameters();
                p.InputFilename = vm.InputIfcFilePath;
                p.OutputFilename = (vm.WriteIfc && !String.IsNullOrWhiteSpace(vm.OutputIfcFilePath)) ? vm.OutputIfcFilePath : null;
                if (vm.SbElementFilter != null) { p.ElementGuidFilter = vm.SbElementFilter.Split(' '); }
                if (vm.SbSpaceFilter != null) { p.SpaceGuidFilter = vm.SbSpaceFilter.Split(' '); }
                p.Flags = (Sbt.EntryPoint.SbtFlags)Convert.ToInt32(vm.Flags, 16);
                p.NotifyMessage = msg => ReportProgress(msg);
                p.WarnMessage = msg => ReportProgress(msg, ProgressEvent.ProgressEventType.Warning);
                p.ErrorMessage = msg => ReportProgress(msg, ProgressEvent.ProgressEventType.Error);
                return p;
            };
            PerformLongOperation = p =>
            {
                try
                {
                    var threadId = GetCurrentWin32ThreadId();
                    ProcessThread thisThread = null;
                    var proc = System.Diagnostics.Process.GetCurrentProcess();
                    foreach (ProcessThread thread in proc.Threads)
                    {
                        if (thread.Id == threadId) { thisThread = thread; }
                    }

                    IList<Sbt.CoreTypes.ElementInfo> elements;
                    IList<Tuple<double, double, double>> compositeDirs;
                    ICollection<Sbt.CoreTypes.SpaceInfo> spaces;
                    IList<Sbt.CoreTypes.SpaceBoundary> spaceBoundaries;
                    float[] correctedSBAreas;
                    int pointCount;
                    int edgeCount;
                    int faceCount;
                    int solidCount;

                    TimeSpan? startCpuTime = null;
                    if (thisThread != null) {
                        startCpuTime = thisThread.TotalProcessorTime; 
                    }
                    DateTime startWallTime = DateTime.Now;
                    Sbt.EntryPoint.CalculateSpaceBoundariesFromIfc(
                        p.InputFilename,
                        p.OutputFilename,
                        out elements,
                        out compositeDirs,
                        out spaces,
                        out spaceBoundaries,
                        out correctedSBAreas,
                        out pointCount,
                        out edgeCount,
                        out faceCount,
                        out solidCount,
                        p.Flags,
                        0.5,
                        p.SpaceGuidFilter,
                        p.ElementGuidFilter,
                        p.NotifyMessage,
                        p.WarnMessage,
                        p.ErrorMessage);
                    TimeSpan? cpuTime = null;
                    if (thisThread != null)
                    {
                        var endTime = thisThread.TotalProcessorTime;
                        cpuTime = endTime - startCpuTime.Value;
                    }
                    TimeSpan wallTime = DateTime.Now - startWallTime;

                    System.Diagnostics.Debug.Assert(
                        !spaceBoundaries.Any(_ => true) ||
                        spaceBoundaries
                        .SelectMany(sb => sb.MaterialLayers)
                        .Select(layer => layer.Id).Max() - 1 <
                        compositeDirs.Count);
                    p.NotifyMessage(String.Format(
                        "Space boundary calculation completed in {0}.\n", 
                        GenerateTimeString(wallTime)));
                    SbtBuildingInformation resultingBuilding = new SbtBuildingInformation();
                    resultingBuilding.IfcFilename = p.InputFilename;
                    resultingBuilding.Elements = new List<Sbt.CoreTypes.ElementInfo>(elements);
                    resultingBuilding.Spaces = new List<Sbt.CoreTypes.SpaceInfo>(spaces);
                    resultingBuilding.SpaceBoundaries = new SpaceBoundaryCollection(spaceBoundaries);
                    resultingBuilding.CompositeDirections = compositeDirs;
                    resultingBuilding.PointCount = pointCount;
                    resultingBuilding.EdgeCount = edgeCount;
                    resultingBuilding.FaceCount = faceCount;
                    resultingBuilding.SolidCount = solidCount;
                    resultingBuilding.CalculationTime = cpuTime;
                    var corrections = new Dictionary<string, float>();
                    for (int i = 0; i < spaceBoundaries.Count; ++i)
                    {
                        if (correctedSBAreas[i] >= 0.0f)
                        {
                            var guid = spaceBoundaries[i].Guid;
                            corrections[guid] = correctedSBAreas[i];
                        }
                    }
                    resultingBuilding.CorrectedAreas = corrections;
                    return resultingBuilding;
                }
                catch (Exception ex)
                {
                    ReportProgress(ex.Message, ProgressEvent.ProgressEventType.Error);
                    return null;
                }
            };
            ProgressHandler = evt => vm.UpdateOutputDirectly(evt.Message);
            LongOperationComplete = res =>
            {
                if (res != null)
                {
                    vm.CurrentSbtBuilding = res;
                    var currProc = Process.GetCurrentProcess();
                    vm.LastPeakWorkingSet = currProc.PeakWorkingSet64;
                    var calcTime = res.CalculationTime;
                    if (calcTime.HasValue)
                    {
                        vm.LastSBCalcTime = GenerateTimeString(calcTime.Value);
                    }
                    else { vm.LastSBCalcTime = String.Empty; }
                }
                completionAction();
            };
        }
    }
}
