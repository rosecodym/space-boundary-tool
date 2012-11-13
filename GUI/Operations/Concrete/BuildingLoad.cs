using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;

using IfcBuildingInformation = IfcInformationExtractor.BuildingInformation;
using IfcSpace = IfcInformationExtractor.Space;

namespace GUI.Operations
{
    class BuildingLoad : Operation<string, IfcBuildingInformation>
    {
        public BuildingLoad(ViewModel vm, Action completionAction)
            : base(_ => vm.UpdateGlobalStatus(), () => vm.ReasonForDisabledIfcModelLoad == null)
        {
            PrepareParameters = () => vm.InputIfcFilePath;
            PerformLongOperation = path =>
            {
                if (path == null) { return null; }
                IfcBuildingInformation res = null;
                ReportProgress("Unpacking schema to load IFC information." + Environment.NewLine);
                string tempPath = System.IO.Path.GetTempFileName();
                try
                {
                    using (System.IO.StreamWriter schemaWriter = new System.IO.StreamWriter(tempPath))
                    {
                        schemaWriter.Write(Properties.Resources.IFC2X3_final);
                    }
                    ReportProgress("Schema unpacked." + Environment.NewLine);
                    using (IfcInformationExtractor.EdmSession edm = new IfcInformationExtractor.EdmSession(tempPath, msg => ReportProgress(msg)))
                    {
                        edm.LoadIfcFile(path);
                        ReportProgress("IFC information loaded." + Environment.NewLine);
                        res = edm.GetBuildingInformation();
                        var multiAssignments =
                            res.SpacesByGuid.Values.Where(s => 
                                s.Zones.Count > 1);
                        foreach (IfcSpace s in multiAssignments)
                        {
                            ReportProgress(
                                String.Format(
                                    "Space {0} is assigned to more than one " +
                                    "zone. All of its zone assignments will " +
                                    "be ignored.",
                                    s.Guid),
                                ProgressEvent.ProgressEventType.Warning);
                            s.Zones.Clear();
                        }
                    }
                }
                finally
                {
                    System.IO.File.Delete(tempPath);
                }
                return res;
            };
            ProgressHandler = evt => vm.UpdateOutputDirectly(evt.Message);
            LongOperationComplete = res =>
            {
                vm.CurrentIfcBuilding = res ?? vm.CurrentIfcBuilding;
                completionAction();
            };
        }
    }
}
