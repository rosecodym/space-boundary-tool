using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;

using Normal = System.Tuple<double, double, double>;

namespace GUI.Operations
{
    class SbtInvocation : Operation<SbtInvocation.Parameters, SbtBuildingInformation>
    {
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
                    IList<Sbt.CoreTypes.ElementInfo> elements;
                    IList<Tuple<double, double, double>> compositeDirs;
                    ICollection<Sbt.CoreTypes.SpaceInfo> spaces;
                    ICollection<Sbt.CoreTypes.SpaceBoundary> spaceBoundaries;
                    var startTime = System.DateTime.Now;
                    int pointCount;
                    int edgeCount;
                    int faceCount;
                    int solidCount;
                    Sbt.EntryPoint.CalculateSpaceBoundariesFromIfc(
                        p.InputFilename,
                        p.OutputFilename,
                        out elements,
                        out compositeDirs,
                        out spaces,
                        out spaceBoundaries,
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
                    System.Diagnostics.Debug.Assert(
                        !spaceBoundaries.Any(_ => true) ||
                        spaceBoundaries
                        .SelectMany(sb => sb.MaterialLayers)
                        .Select(layer => layer.Id).Max() - 1 <
                        compositeDirs.Count);
                    p.NotifyMessage("Space boundary calculation completed in " + GenerateTimeString(DateTime.Now - startTime) + ".\n");
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
                vm.CurrentSbtBuilding = res ?? vm.CurrentSbtBuilding;
                var currProc = System.Diagnostics.Process.GetCurrentProcess();
                vm.LastPeakWorkingSet = currProc.PeakWorkingSet64;
                var currTime = currProc.TotalProcessorTime;
                var diff = currTime - startCpuTime;
                vm.LastSBCalcTime = GenerateTimeString(diff);
                completionAction();
            };
        }
    }
}
