using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Windows.Input;

using Microsoft.Win32;

namespace GUI
{
    static class Commands
    {
        static public void BrowseToInputIfcFile(ViewModel vm)
        {
            OpenFileDialog ofd = new OpenFileDialog();
            ofd.Filter = "IFC files|*.ifc";
            bool? result = ofd.ShowDialog();
            if (result.HasValue && result.Value == true)
            {
                vm.InputIfcFilePath = ofd.FileName;
            }
        }

        static public void BrowseToOutputIfcFile(ViewModel vm)
        {
            SaveFileDialog sfd = new SaveFileDialog();
            sfd.Filter = "IFC files|*.ifc";
            bool? result = sfd.ShowDialog();
            if (result.HasValue && result.Value == true)
            {
                vm.OutputIfcFilePath = sfd.FileName;
            }
        }

        class SbtParameters
        {
            public string InputFilename { get; set; }
            public string OutputFilename { get; set; }
            public Sbt.EntryPoint.SbtFlags Flags { get; set; }
            public Sbt.EntryPoint.MessageDelegate NotifyMessage { get; set; }
            public Sbt.EntryPoint.MessageDelegate WarnMessage { get; set; }
            public Sbt.EntryPoint.MessageDelegate ErrorMessage { get; set; }
        }

        static public void InvokeSbt(ViewModel vm)
        {
            if (!vm.Busy)
            {
                try
                {
                    vm.Busy = true;
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
                        BuildingInformation res = e.Result as BuildingInformation;
                        if (res != null) { vm.CurrentBuilding = res; }
                        vm.Busy = false;
                    });

                    SbtParameters p = new SbtParameters();
                    p.InputFilename = vm.InputIfcFilePath;
                    p.OutputFilename = vm.OutputIfcFilePath;
                    p.Flags = Sbt.EntryPoint.SbtFlags.SkipWallSlabCheck;
                    p.NotifyMessage = p.WarnMessage = p.ErrorMessage = (msg) => worker.ReportProgress(0, msg);

                    vm.SelectedTabIndex = 2;
                    worker.RunWorkerAsync(p);
                }
                catch (Exception)
                {
                    vm.Busy = false;
                }
            }
        }

        static public void GenerateIdf(ViewModel vm)
        {
            throw new NotImplementedException();
        }

        static void DoSbtWork(object sender, DoWorkEventArgs e)
        {
            SbtParameters p = e.Argument as SbtParameters;
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
                    0,
                    p.NotifyMessage,
                    p.WarnMessage,
                    p.ErrorMessage);
                BuildingInformation resultingBuilding = new BuildingInformation();
                resultingBuilding.IfcFilename = p.InputFilename;
                resultingBuilding.Elements = new List<Sbt.CoreTypes.ElementInfo>(elements);
                resultingBuilding.Spaces = new List<Sbt.CoreTypes.SpaceInfo>(spaces);
                resultingBuilding.SpaceBoundaries = new SpaceBoundaryCollection(spaceBoundaries);
                e.Result = resultingBuilding;
            }
        }
    }
}
