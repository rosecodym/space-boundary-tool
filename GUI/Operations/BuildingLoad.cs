using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;

using IfcBuildingInformation = IfcInformationExtractor.BuildingInformation;

namespace GUI.Operations
{
    static class BuildingLoad
    {
        class Parameters
        {
            public string Path { get; set; }
            public Action<string> Notify { get; set; }
        }

        static public void Execute(ViewModel vm)
        {
            if (!vm.CurrentlyLoadingIfcModel)
            {
                try
                {
                    vm.CurrentlyLoadingIfcModel = true;
                    BackgroundWorker worker = new BackgroundWorker();
                    worker.WorkerReportsProgress = true;
                    worker.DoWork += new DoWorkEventHandler(DoBuildingLoadWork);
                    worker.ProgressChanged += new ProgressChangedEventHandler((sender, e) =>
                    {
                        string msg = e.UserState as string;
                        if (msg != null) { vm.UpdateOutputDirectly(msg); }
                    });
                    worker.RunWorkerCompleted += new RunWorkerCompletedEventHandler((sender, e) =>
                    {
                        var res = e.Result as IfcBuildingInformation;
                        if (res != null) { vm.CurrentIfcBuilding = res; }
                        vm.CurrentlyLoadingIfcModel = false;
                    });

                    Parameters p = new Parameters();
                    p.Path = vm.InputIfcFilePath;
                    p.Notify = msg => worker.ReportProgress(0, msg);

                    worker.RunWorkerAsync(p);
                }
                catch (Exception)
                {
                    vm.CurrentlyLoadingIfcModel = false;
                }
            }
        }

        static void DoBuildingLoadWork(object sender, DoWorkEventArgs e)
        {
            Parameters p = e.Argument as Parameters;
            if (p != null)
            {
                p.Notify("Unpacking schema to load IFC information." + Environment.NewLine);
                string tempPath = System.IO.Path.GetTempFileName();
                try
                {
                    using (System.IO.StreamWriter schemaWriter = new System.IO.StreamWriter(tempPath))
                    {
                        schemaWriter.Write(Properties.Resources.IFC2X3_final);
                    }
                    p.Notify("Schema unpacked." + Environment.NewLine);
                    using (IfcInformationExtractor.EdmSession edm = new IfcInformationExtractor.EdmSession(tempPath, p.Notify))
                    {
                        edm.LoadIfcFile(p.Path);
                        e.Result = edm.GetBuildingInformation();
                        p.Notify("IFC information loaded." + Environment.NewLine);
                    }
                }
                finally
                {
                    System.IO.File.Delete(tempPath);
                }
            }
        }
    }
}
