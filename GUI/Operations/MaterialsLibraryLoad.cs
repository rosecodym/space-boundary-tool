using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Text;

using LibIdf.Idd;
using LibIdf.Idf;

using MaterialLibraryEntry = GUI.Materials.LibraryEntries.Opaque;

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

        static public void Execute(ViewModel vm)
        {
            if (!vm.CurrentlyLoadingMaterialLibrary && !String.IsNullOrWhiteSpace(vm.MaterialsLibraryPath))
            {
                try
                {
                    vm.CurrentlyLoadingMaterialLibrary = true;
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
                        vm.CurrentlyLoadingMaterialLibrary = false;
                    });

                    Parameters p = new Parameters();
                    p.Path = vm.MaterialsLibraryPath;
                    p.Idds = vm.Idds;
                    p.Notify = msg => worker.ReportProgress(0, msg);

                    worker.RunWorkerAsync(p);
                }
                catch (Exception)
                {
                    vm.CurrentlyLoadingMaterialLibrary = false;
                }
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

                List<Materials.LibraryEntries.Opaque> res = new List<Materials.LibraryEntries.Opaque>();
                HashSet<IdfObject> objs;

                objs = idf.GetObjectsByType("Material", false);
                foreach (IdfObject obj in objs)
                {
                    try
                    {
                        res.Add(new Materials.LibraryEntries.Opaque(obj));
                    }
                    catch (Exception)
                    {
                        string objName = String.IsNullOrWhiteSpace(obj.Name) ? "<unnamed-object>" : obj.Name;
                        p.Notify(String.Format("Warning: Failed to load material library object '{0}'. Check the definition in the IDF.\n", objName));
                    }
                }

                p.Notify("Found " + res.Count.ToString() + " materials.\n");
                e.Result = res;
            }
        }
    }
}
