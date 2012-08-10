using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;

namespace GUI.Operations
{
    static partial class IdfGeneration
    {
        static public void Execute(ViewModel vm)
        {
            if (!vm.Busy)
            {
                try
                {
                    vm.Busy = true;
                    // TODO: check for pre-existing building
                    BackgroundWorker worker = new BackgroundWorker();
                    worker.WorkerReportsProgress = true;
                    worker.DoWork += new DoWorkEventHandler(DoIdfGenerationWork);
                    worker.ProgressChanged += new ProgressChangedEventHandler((sender, e) =>
                    {
                        string msg = e.UserState as string;
                        if (msg != null) { vm.UpdateOutputDirectly(msg); }
                    });
                    worker.RunWorkerCompleted += new RunWorkerCompletedEventHandler((sender, e) =>
                    {
                        vm.Busy = false;
                    });

                    Parameters p = new Parameters();
                    p.OutputFilename = vm.OutputIdfFilePath;
                    p.Building = vm.CurrentBuilding;
                    p.GetIdd = () => vm.Idds.GetIddFor((EnergyPlusVersion)vm.EnergyPlusVersionIndexToWrite, msg => worker.ReportProgress(0, msg + Environment.NewLine));
                    p.Notify = msg => worker.ReportProgress(0, msg);

                    vm.SelectedTabIndex = 2;
                    worker.RunWorkerAsync(p);
                }
                catch (Exception)
                {
                    vm.Busy = false;
                }
            }
        }

        static void DoIdfGenerationWork(object sender, DoWorkEventArgs e)
        {
            Parameters p = e.Argument as Parameters;
            if (p != null)
            {
                try
                {
                    p.Notify("Getting IDD.\n");
                    LibIdf.Idd.Idd idd = p.GetIdd();
                    p.Notify("Got IDD.\n");
                    IdfCreator creator = IdfCreator.Build(p.EPVersion, idd, p.Notify);
                    creator.AddConstantContents();
                    creator.WriteToFile(p.OutputFilename);
                    p.Notify("IDF written.\n");
                }
                catch (Exception ex)
                {
                    p.Notify("Operation failed: " + ex.Message + Environment.NewLine);
                }
            }
        }
    }
}
