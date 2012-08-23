using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;

using Microsoft.Win32;

using MaterialLibraryEntry = GUI.Materials.LibraryEntries.Opaque;

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
                vm.OutputIfcFilePath = Path.Combine(Path.GetDirectoryName(ofd.FileName), Path.GetFileNameWithoutExtension(ofd.FileName) + "-sb.ifc");
                vm.OutputIdfFilePath = Path.ChangeExtension(ofd.FileName, "idf");
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

        static public void LinkConstructions(MaterialLibraryEntry idfConstruction, IEnumerable<IfcConstruction> ifcConstructions)
        {
            if (idfConstruction != null && ifcConstructions != null)
            {
                foreach (IfcConstruction c in ifcConstructions)
                {
                    c.IdfMappingTarget = idfConstruction;
                }
            }
        }

        static public void ViewIdf(string idfPath)
        {
            if (File.Exists(idfPath))
            {
                System.Diagnostics.Process.Start(idfPath);
            }
        }
    }
}
