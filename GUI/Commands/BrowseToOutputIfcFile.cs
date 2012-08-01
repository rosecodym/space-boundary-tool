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
        static public void BrowseToOutputIfcFile(ViewModel vm)
        {
            SaveFileDialog sfd = new SaveFileDialog();
            bool? result = sfd.ShowDialog();
            if (result.HasValue && result.Value == true)
            {
                vm.OutputIfcFilePath = sfd.FileName;
            }
        }
    }
}
