using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Input;

namespace GUI.Commands
{
    class BrowseForFileToOpen : ICommand
    {
        public bool CanExecute(object parameter)
        {
            return false;
        }

        public event EventHandler CanExecuteChanged;

        public void Execute(object parameter)
        {
            throw new NotImplementedException();
        }
    }
}
