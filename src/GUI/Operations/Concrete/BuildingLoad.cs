using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;

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
                try
                {
                    return new IfcBuildingInformation(path);
                }
                catch (Exception e)
                {
                    ReportProgress(
                        e.Message,
                        ProgressEvent.ProgressEventType.Error);
                    return null;
                }
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
