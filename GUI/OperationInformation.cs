using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace GUI
{
    class OperationInformation
    {
        readonly Action<Action, Action> operate;
        readonly Action inProgressChanged;

        public bool InProgress { get; private set; }

        public OperationInformation(Action<ViewModel, Action, Action> operation, ViewModel vm, Action inProgressChanged)
        {
            this.operate = (begin, end) => { operation(vm, begin, end); };
            this.inProgressChanged = inProgressChanged;
            this.InProgress = false;
        }

        public void Operate()
        {
            Action begin = () => { InProgress = true; inProgressChanged(); };
            Action end = () => { InProgress = false; inProgressChanged(); };
            if (!InProgress)
            {
                operate(begin, end);
            }
        }
    }
}
