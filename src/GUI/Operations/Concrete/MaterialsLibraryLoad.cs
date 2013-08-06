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
                    foreach (var w in warnings)
                    {
                        var msg = 
                            "Materials library warning: " 
                            + w 
                            + Environment.NewLine;
                        ReportProgress(msg, ProgressEventType.Warning);
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
            Func<Field, string> stringify = f =>
            {
                if (f.IdfObject.Name == null)
                {
                    return String.Format(
                        "Field '{0}' of unnamed '{1}' object",
                        f.Name == null ? f.Code : f.Name,
                        f.IdfObject.Type);
                }
                else
                {
                    return String.Format(
                        "Field '{0}' of '{1}' object '{2}'",
                        f.Name == null ? f.Code : f.Name,
                        f.IdfObject.Type,
                        f.IdfObject.Name);
                }
            };

            callbacks.AlphaIntoNumeric = (f, v) =>
            {
                warnings.Add(String.Format(
                    "{0} has the alphanumeric value '{1}', but is numeric.", 
                    stringify(f), 
                    v));
                return IdfToolbox.Base.FieldValue.Nothing;
            };

            callbacks.ImproperAutocalculate = f =>
            {
                warnings.Add(String.Format(
                    "{0} has an illegal 'Autocalculate' value.",
                    stringify(f)));
                return IdfToolbox.Base.FieldValue.Nothing;
            };

            callbacks.ImproperAutosize = f =>
            {
                warnings.Add(String.Format(
                    "{0} has an illegal 'Autosize' value.",
                    stringify(f)));
                return IdfToolbox.Base.FieldValue.Nothing;
            };

            callbacks.IntegerOutOfBounds = (f, v) =>
            {
                warnings.Add(String.Format(
                    "{0} has an out-of-bounds value '{1}'.",
                    stringify(f),
                    v));
                return IdfToolbox.Base.FieldValue.Nothing;
            };

            callbacks.InvalidKey = (f, v) =>
            {
                warnings.Add(String.Format(
                    "{0} has the non-key value '{1}'.",
                    stringify(f),
                    v));
                return IdfToolbox.Base.FieldValue.Nothing;
            };

            callbacks.RealIntoInteger = (f, v) =>
            {
                warnings.Add(String.Format(
                    "{0} has real value '{1}', but is an integer field.",
                    stringify(f),
                    v));
                return IdfToolbox.Base.FieldValue.Nothing;
            };

            callbacks.RealOutOfBounds = (f, v) =>
            {
                warnings.Add(String.Format(
                    "{0} has an out-of-bounds value '{1}'.",
                    stringify(f),
                    v));
                return IdfToolbox.Base.FieldValue.Nothing;
            };

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
