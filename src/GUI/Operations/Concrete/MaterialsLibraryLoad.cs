using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Text;

using IdfToolbox.Idd;
using IdfToolbox.Idf;

using MaterialLibraryEntry = ConstructionManagement.MaterialLibrary.LibraryEntry;
using ProgressEventType = GUI.Operations.ProgressEvent.ProgressEventType;

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
                    var warnings = new List<string>();
                    Idf idf = new Idf(p.Path, idd, CreateLogger(warnings));
                    Action<string> warn = w =>
                        ReportProgress(
                            "Materials library warning: " 
                                + w 
                                + Environment.NewLine,
                            ProgressEventType.Warning);
                    foreach (var w in warnings)
                    {
                        warn(w);
                    }
                    foreach (var obj in idf.GetAllObjects())
                    {
                        foreach (var f in obj.Fields)
                        {
                            if (f.Value.IsNothing && 
                                f.IddInfo.Required &&
                                f.Name != "Thickness")
                            {
                                if (obj.Name == null)
                                {
                                    warn(String.Format(
                                        "Field '{0}' of unnamed '{1}' object has a missing or invalid value.", 
                                        f.Name == null ? f.Code : f.Name, 
                                        f.IdfObject.Type));
                                }
                                else
                                {
                                    warn(String.Format(
                                        "Field '{0}' of '{1}' object '{2}' has a missing or invalid value.", 
                                        f.Name == null ? f.Code : f.Name, 
                                        f.IdfObject.Type, 
                                        f.IdfObject.Name));
                                }
                            }
                        }
                    }
                    ReportProgress("IDF loaded.\n");

                    res = ConstructionManagement.MaterialLibrary.Load(idf, msg => ReportProgress(msg));

                    ReportProgress("Found " + res.Count.ToString() + " materials.\n");
                }
                catch (Exception)
                {
                    ReportProgress(
                        "IDF loading failed!" + Environment.NewLine, 
                        ProgressEventType.Error);
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

        ValidationCallbackTable CreateLogger(IList<string> warnings)
        {
            var callbacks = Idf.CreateDefaultLoadCallbacks();
            callbacks.UnknownObjectType = (attempt, _fieldValues) =>
            {
                warnings.Add(String.Format(
                    "'{0}' is not a legal object type.",
                    attempt));
                return null;
            };
            return callbacks;
        }
    }
}
