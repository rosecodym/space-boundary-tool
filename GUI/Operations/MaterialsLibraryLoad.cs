﻿using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Text;

using LibIdf.Idd;
using LibIdf.Idf;

using MaterialLibraryEntry = ConstructionManagement.MaterialLibrary.LibraryEntry;

namespace GUI.Operations
{
    static class MaterialsLibraryLoad
    {
        class Parameters
        {
            public string Path { get; set; }
            public IddManager Idds { get; set; }
            public Action<string> Notify { get; set; }
        }

        static public void Execute(ViewModel vm, Action begin, Action end)
        {
            if (!String.IsNullOrWhiteSpace(vm.MaterialsLibraryPath))
            {
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
                    var res = e.Result as ICollection<MaterialLibraryEntry>;
                    if (res != null) { vm.LibraryMaterials = res; }
                    end();
                });

                Parameters p = new Parameters();
                p.Path = vm.MaterialsLibraryPath;
                p.Idds = vm.Idds;
                p.Notify = msg => worker.ReportProgress(0, msg);

                begin();
                worker.RunWorkerAsync(p);
            }
        }

        static void DoLoadMaterialsLibraryWork(object sender, DoWorkEventArgs e)
        {
            Parameters p = e.Argument as Parameters;
            if (p != null)
            {
                string versionGuess = Idf.GuessVersion(p.Path);
                EnergyPlusVersion ver =
                    versionGuess == "7.1" ? EnergyPlusVersion.V710 : EnergyPlusVersion.V710;
                p.Notify("Treating materials library as IDD version " + ver.ToString() + ".\n");

                p.Notify("Loading IDD...\n");
                Idd idd = p.Idds.GetIddFor(ver, msg => p.Notify(msg + Environment.NewLine));
                p.Notify("IDD loaded. Loading IDF...\n");
                Idf idf = new Idf(p.Path, idd);
                p.Notify("IDF loaded.\n");

                var res = ConstructionManagement.MaterialLibrary.Load(idf, p.Notify);

                p.Notify("Found " + res.Count.ToString() + " materials.\n");
                e.Result = res;
            }
        }
    }
}
