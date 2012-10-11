using System;
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
    class MaterialsLibraryLoad : Operation<MaterialsLibraryLoad.Parameters, ISet<MaterialLibraryEntry>>
    {
        public class Parameters
        {
            public string Path { get; set; }
            public IddManager Idds { get; set; }
        }

        public MaterialsLibraryLoad(ViewModel vm, Action completionAction)
            : base(_ => vm.UpdateGlobalStatus())
        {
            PrepareParameters = () =>
            {
                Parameters p = new Parameters();
                p.Path = vm.MaterialsLibraryPath;
                p.Idds = vm.Idds;
                return p;
            };
            PerformLongOperation = p =>
            {
                string versionGuess = Idf.GuessVersion(p.Path);
                EnergyPlusVersion ver =
                    versionGuess == "7.1" ? EnergyPlusVersion.V710 : EnergyPlusVersion.V710;
                ReportProgress("Treating materials library as IDD version " + ver.ToString() + ".\n");

                ReportProgress("Loading IDD...\n");
                Idd idd = p.Idds.GetIddFor(ver, msg => ReportProgress(msg + Environment.NewLine));
                ReportProgress("IDD loaded. Loading IDF...\n");
                Idf idf = new Idf(p.Path, idd);
                ReportProgress("IDF loaded.\n");

                var res = ConstructionManagement.MaterialLibrary.Load(idf, msg => ReportProgress(msg));

                ReportProgress("Found " + res.Count.ToString() + " materials.\n");
                return res;
            };
            ProgressHandler = evt => vm.UpdateOutputDirectly(evt.Message);
            LongOperationComplete = res =>
            {
                vm.LibraryMaterials = res;
                completionAction();
            };
        }
    }
}
