using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;

using IfcBuildingInformation = IfcInformationExtractor.BuildingInformation;

namespace GUI.Operations
{
    class BuildingLoad : Operation<string, IfcBuildingInformation>
    {
        public BuildingLoad(ViewModel vm)
            : base(_ => vm.UpdateGlobalStatus())
        {
            PrepareParameters = () => vm.InputIfcFilePath;
            PerformLongOperation = path =>
            {
                IfcBuildingInformation res = null;
                if (path != null)
                {
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
                        }
                    }
                    finally
                    {
                        System.IO.File.Delete(tempPath);
                    }
                }
                return res;
            };
            ProgressHandler = evt => vm.UpdateOutputDirectly(evt.Message);
            LongOperationComplete = res => vm.CurrentIfcBuilding = res ?? vm.CurrentIfcBuilding;
        }
    }
}
