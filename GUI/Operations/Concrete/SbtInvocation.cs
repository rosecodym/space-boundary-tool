﻿using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;

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
            : base(_ => vm.UpdateGlobalStatus())
        {
            PrepareParameters = () =>
            {
                Parameters p = new Parameters();
                p.InputFilename = vm.InputIfcFilePath;
                p.OutputFilename = (vm.WriteIfc && !String.IsNullOrWhiteSpace(vm.OutputIfcFilePath)) ? vm.OutputIfcFilePath : null;
                if (vm.SbElementFilter != null) { p.ElementGuidFilter = vm.SbElementFilter.Split(' '); }
                if (vm.SbSpaceFilter != null) { p.SpaceGuidFilter = vm.SbSpaceFilter.Split(' '); }
                p.Flags = (Sbt.EntryPoint.SbtFlags)Convert.ToInt32(vm.Flags, 16);
                p.NotifyMessage = p.WarnMessage = p.ErrorMessage = msg => ReportProgress(msg);
                return p;
            };
            PerformLongOperation = p =>
            {
                try
                {
                    IList<Sbt.CoreTypes.ElementInfo> elements;
                    ICollection<Sbt.CoreTypes.SpaceInfo> spaces;
                    ICollection<Sbt.CoreTypes.SpaceBoundary> spaceBoundaries;
                    var startTime = System.DateTime.Now;
                    Sbt.EntryPoint.CalculateSpaceBoundariesFromIfc(
                        p.InputFilename,
                        p.OutputFilename,
                        out elements,
                        out spaces,
                        out spaceBoundaries,
                        p.Flags,
                        0.01,
                        0.5,
                        p.SpaceGuidFilter,
                        p.ElementGuidFilter,
                        p.NotifyMessage,
                        p.WarnMessage,
                        p.ErrorMessage);
                    p.NotifyMessage("Space boundary calculation completed in " + GenerateTimeString(DateTime.Now - startTime) + ".\n");
                    SbtBuildingInformation resultingBuilding = new SbtBuildingInformation();
                    resultingBuilding.IfcFilename = p.InputFilename;
                    resultingBuilding.Elements = new List<Sbt.CoreTypes.ElementInfo>(elements);
                    resultingBuilding.Spaces = new List<Sbt.CoreTypes.SpaceInfo>(spaces);
                    resultingBuilding.SpaceBoundaries = new SpaceBoundaryCollection(spaceBoundaries);
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
                completionAction();
            };
        }
    }
}
