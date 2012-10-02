using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace GUI
{
    class OperationInformation
    {
        readonly Action<Action, Action> operate;

        public bool InProgress { get; private set; }

        public OperationInformation(Action<ViewModel, Action, Action> operation, ViewModel vm)
        {
            this.operate = (begin, end) => { operation(vm, begin, end); };
            this.InProgress = false;
        }

        public void Operate()
        {
            Action begin = () => InProgress = true;
            Action end = () => InProgress = false;
            if (!InProgress)
            {
                operate(begin, end);
            }
        }
    }
}
