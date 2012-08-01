using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Input;

using Microsoft.Win32;

namespace GUI
{
    static partial class Commands
    {
        static public void BrowseToInputIfcFile(ViewModel vm)
        {
            OpenFileDialog ofd = new OpenFileDialog();
            bool? result = ofd.ShowDialog();
            if (result.HasValue && result.Value == true)
            {
                vm.InputIfcFilePath = ofd.FileName;
            }
        }
    }
}
