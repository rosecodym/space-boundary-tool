using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using Microsoft.Win32;

namespace GUI.Operations
{
    static class Miscellaneous
    {
        static public void BrowseToInputIfcFile(ViewModel vm)
        {
            OpenFileDialog ofd = new OpenFileDialog();
            ofd.Filter = "IFC files|*.ifc";
            bool? result = ofd.ShowDialog();
            if (result.HasValue && result.Value == true)
            {
                vm.InputIfcFilePath = ofd.FileName;
            }
        }

        static public void BrowseToOutputIfcFile(ViewModel vm)
        {
            SaveFileDialog sfd = new SaveFileDialog();
            sfd.Filter = "IFC files|*.ifc";
            bool? result = sfd.ShowDialog();
            if (result.HasValue && result.Value == true)
            {
                vm.OutputIfcFilePath = sfd.FileName;
            }
        }

        static public void BrowseToOutputIdfFile(ViewModel vm)
        {
            SaveFileDialog sfd = new SaveFileDialog();
            sfd.Filter = "IDF files|*.idf";
            bool? result = sfd.ShowDialog();
            if (result.HasValue && result.Value == true)
            {
                vm.OutputIdfFilePath = sfd.FileName;
            }
        }

        static public void BrowseToMaterialsLibrary(ViewModel vm)
        {
            OpenFileDialog ofd = new OpenFileDialog();
            ofd.Filter = "IDF files|*.idf";
            bool? result = ofd.ShowDialog();
            if (result.HasValue && result.Value == true)
            {
                vm.MaterialsLibraryPath = ofd.FileName;
            }
        }

        static public void LinkCurrentlySelectedConstructions(ViewModel vm)
        {
            if (vm.SelectedIdfConstruction != null && vm.SelectedIfcConstruction != null)
            {
                vm.SelectedIfcConstruction.IdfMappingTarget = vm.SelectedIdfConstruction.Name;
            }
        }
    }
}
