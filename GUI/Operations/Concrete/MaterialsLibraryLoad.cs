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
            : base(_ => vm.UpdateGlobalStatus(), () => vm.ReasonForDisabledMaterialLibraryLoad == null)
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
                ISet<MaterialLibraryEntry> res = null;
                try
                {
                    string versionGuess = Idf.GuessVersion(p.Path);
                    var epv = IddManager.StringToVersion(versionGuess);

                    ReportProgress("Loading IDD...\n");
                    Idd idd = p.Idds.GetIddFor(epv, msg => ReportProgress(msg + Environment.NewLine));
                    ReportProgress("IDD loaded. Loading IDF...\n");
                    Idf idf = new Idf(p.Path, idd);
                    ReportProgress("IDF loaded.\n");

                    res = ConstructionManagement.MaterialLibrary.Load(idf, msg => ReportProgress(msg));

                    ReportProgress("Found " + res.Count.ToString() + " materials.\n");
                }
                catch (Exception)
                {
                    ReportProgress("IDF loading failed!" + Environment.NewLine, ProgressEvent.ProgressEventType.Error);
                }
                return res;
            };
            ProgressHandler = evt =>
            {
                vm.UpdateOutputDirectly(evt.Message);
            };
            LongOperationComplete = res =>
            {
                vm.LibraryMaterials = res;
                completionAction();
            };
        }
    }
}
