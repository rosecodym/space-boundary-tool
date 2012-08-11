using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;

namespace GUI.Operations
{
    static class LoadMaterialsLibrary
    {
        class Parameters
        {
            public string Path { get; set; }
            public Action<string> Notify { get; set; }
        }

        static public void Execute(ViewModel vm)
        {
            if (!vm.Busy)
            {
                try
                {
                    vm.Busy = true;
                    BackgroundWorker worker = new BackgroundWorker();
                    worker.WorkerReportsProgress = true;
                    worker.DoWork += new DoWorkEventHandler(DoLoadMaterialsLibraryWork);
                    worker.ProgressChanged += new ProgressChangedEventHandler((sender, e) =>
                    {
                        string msg = e.UserState as string;
                        if (msg != null) { vm.UpdateOutputDirectly(msg); }
                    });
                    worker.RunWorkerCompleted += new RunWorkerCompletedEventHandler((sender, e) =>
                    {
                        var res = e.Result as ICollection<Constructions.MaterialLayer>;
                        if (res != null) { vm.LibraryMaterials = res; }
                        vm.Busy = false;
                    });

                    Parameters p = new Parameters();
                    p.Path = vm.MaterialsLibraryPath;
                    p.Notify = msg => worker.ReportProgress(0, msg);

                    worker.RunWorkerAsync(p);
                }
                catch (Exception)
                {
                    vm.Busy = false;
                }
            }
        }

        static void DoLoadMaterialsLibraryWork(object sender, DoWorkEventArgs e)
        {
            Parameters p = e.Argument as Parameters;
            if (p != null)
            {
                string versionGuess = LibIdf.Idf.Idf.GuessVersion(p.Path);
                EnergyPlusVersion ver =
                    versionGuess == "7.1" ? EnergyPlusVersion.V710 : EnergyPlusVersion.V710;
                p.Notify("Loading materials library as IDD version " + ver.ToString() + ".\n");
                e.Result = new List<Constructions.MaterialLayer>();
            }
        }
    }
}
