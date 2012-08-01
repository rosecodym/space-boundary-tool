using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Input;

namespace GUI
{
    public class DelegateCommand : ICommand
    {
        Action<object> _actionToExecute;

        public DelegateCommand(Action<object> actionToExecute)
        {
            _actionToExecute = actionToExecute;
        }

        public bool CanExecute(object parameter)
        {
            return true;
        }

        public event EventHandler CanExecuteChanged;

        public void Execute(object parameter)
        {
            _actionToExecute(parameter);
        }
    }
}
