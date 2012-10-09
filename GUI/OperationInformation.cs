using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace GUI
{
    class OperationInformation
    {
        readonly ViewModel vm;
        readonly Action<ViewModel, Action, Action> operate;
        readonly Action inProgressChanged;

        bool runEver = false;

        public bool InProgress { get; private set; }
        public OperationStatus Status
        {
            get
            {
                return
                    !runEver ?      OperationStatus.BeforeStart :
                    InProgress ?    OperationStatus.InProgress : 
                                    OperationStatus.OK;
            }
        }

        public OperationInformation(Action<ViewModel, Action, Action> operation, ViewModel vm, Action inProgressChanged)
        {
            this.vm = vm;
            this.operate = operation;
            this.inProgressChanged = inProgressChanged;
            this.InProgress = false;
        }

        public void Operate()
        {
            Action begin = () => { runEver = InProgress = true; inProgressChanged(); vm.UpdateGlobalStatus(); };
            Action end = () => { InProgress = false; inProgressChanged(); vm.UpdateGlobalStatus();  };
            if (!InProgress)
            {
                operate(vm, begin, end);
            }
        }
    }
}
