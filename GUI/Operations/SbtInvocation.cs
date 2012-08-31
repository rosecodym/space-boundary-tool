using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;

namespace GUI.Operations
{
    static class SbtInvocation
    {
        class Parameters
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

        static public void Execute(ViewModel vm)
        {
            if (!vm.CurrentlyCalculatingSBs)
            {
                try
                {
                    vm.CurrentlyCalculatingSBs = true;
                    // TODO: check for pre-existing building
                    BackgroundWorker worker = new BackgroundWorker();
                    worker.WorkerReportsProgress = true;
                    worker.DoWork += new DoWorkEventHandler(DoSbtWork);
                    worker.ProgressChanged += new ProgressChangedEventHandler((sender, e) =>
                    {
                        string msg = e.UserState as string;
                        if (msg != null) { vm.UpdateOutputDirectly(msg); }
                    });
                    worker.RunWorkerCompleted += new RunWorkerCompletedEventHandler((sender, e) =>
                    {
                        SbtBuildingInformation res = e.Result as SbtBuildingInformation;
                        if (res != null) { vm.CurrentSbtBuilding = res; }
                        vm.CurrentlyCalculatingSBs = false;
                    });

                    Parameters p = new Parameters();
                    p.InputFilename = vm.InputIfcFilePath;
                    p.OutputFilename = (vm.WriteIfc && !String.IsNullOrWhiteSpace(vm.OutputIfcFilePath)) ? vm.OutputIfcFilePath : null;

                    if (vm.SbElementFilter != null) { p.ElementGuidFilter = vm.SbElementFilter.Split(' '); }
                    if (vm.SbSpaceFilter != null) { p.SpaceGuidFilter = vm.SbSpaceFilter.Split(' '); }

                    p.Flags = Sbt.EntryPoint.SbtFlags.SkipWallSlabCheck;

                    p.NotifyMessage = p.WarnMessage = p.ErrorMessage = msg => worker.ReportProgress(0, msg);

                    worker.RunWorkerAsync(p);
                }
                catch (Exception)
                {
                    vm.CurrentlyCalculatingSBs = false;
                }
            }
        }

        static void DoSbtWork(object sender, DoWorkEventArgs e)
        {
            Parameters p = e.Argument as Parameters;
            try
            {
                if (p != null)
                {
                    IList<Sbt.CoreTypes.ElementInfo> elements;
                    ICollection<Sbt.CoreTypes.SpaceInfo> spaces;
                    ICollection<Sbt.CoreTypes.SpaceBoundary> spaceBoundaries;
                    Sbt.EntryPoint.CalculateSpaceBoundariesFromIfc(
                        p.InputFilename,
                        p.OutputFilename,
                        out elements,
                        out spaces,
                        out spaceBoundaries,
                        p.Flags,
                        0.01,
                        3.0,
                        p.SpaceGuidFilter,
                        p.ElementGuidFilter,
                        0,
                        p.NotifyMessage,
                        p.WarnMessage,
                        p.ErrorMessage);
                    SbtBuildingInformation resultingBuilding = new SbtBuildingInformation();
                    resultingBuilding.IfcFilename = p.InputFilename;
                    resultingBuilding.Elements = new List<Sbt.CoreTypes.ElementInfo>(elements);
                    resultingBuilding.Spaces = new List<Sbt.CoreTypes.SpaceInfo>(spaces);
                    resultingBuilding.SpaceBoundaries = new SpaceBoundaryCollection(spaceBoundaries);
                    e.Result = resultingBuilding;
                }
            }
            catch (Exception ex)
            {
                p.ErrorMessage(ex.Message);
            }
        }
    }
}
