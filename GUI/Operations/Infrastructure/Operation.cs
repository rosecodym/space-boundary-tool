using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;

namespace GUI.Operations
{
    public enum OperationStatus
    {
        BeforeStart,
        OK,
        InProgress,
        Warnings,
        Errors
    }

    abstract class Operation<TParameters, TResult>
        where TParameters : class
        where TResult : class
    {
        private BackgroundWorker bw = new BackgroundWorker();
        private bool hasRunEver = false;
        private IList<Problem> problems = new List<Problem>();
        private Action<OperationStatus> statusChanged = _ => { };

        protected Action<ProgressEvent> ProgressHandler { private get; set; }
        protected Func<TParameters> PrepareParameters { private get; set; }
        protected Func<TParameters, TResult> PerformLongOperation { private get; set; }
        protected Action<TResult> LongOperationComplete { get; set; }

        public bool InProgress { get; private set; }
        public OperationStatus Status
        {
            get
            {
                return
                    InProgress ? OperationStatus.InProgress :
                    !hasRunEver ? OperationStatus.BeforeStart :
                    problems.Count == 0 ? OperationStatus.OK :
                    problems.Any(p => p.Type == Problem.ProblemType.Error) ? OperationStatus.Errors : OperationStatus.Warnings;
            }
        }
        public ICollection<Problem> Problems { get { return problems; } }

        protected Operation(Action<OperationStatus> statusChanged)
        {
            this.statusChanged = statusChanged;
            InProgress = false;
            bw.WorkerReportsProgress = true;
            bw.DoWork += new DoWorkEventHandler((_, e) => e.Result = PerformLongOperation(e.Argument as TParameters));
            bw.ProgressChanged += new ProgressChangedEventHandler((_sender, e) =>
            {
                ProgressEvent evt = e.UserState as ProgressEvent;
                if (evt != null)
                {
                    if (evt.Type != ProgressEvent.ProgressEventType.Notification)
                    {
                        problems.Add(new Problem(evt.Type == ProgressEvent.ProgressEventType.Warning ? Problem.ProblemType.Warning : Problem.ProblemType.Error, evt.Message));
                    }
                    ProgressHandler(evt);
                    statusChanged(Status);
                }
            });
            bw.RunWorkerCompleted += new RunWorkerCompletedEventHandler((_, e) =>
            {
                hasRunEver = true;
                InProgress = false;
                LongOperationComplete(e.Result as TResult);
                statusChanged(Status);
            });

            ProgressHandler = _ => { };
            PrepareParameters = () => null;
            PerformLongOperation = _ => null;
            LongOperationComplete = _ => { };
        }

        protected void ReportProgress(string message, ProgressEvent.ProgressEventType status = ProgressEvent.ProgressEventType.Notification)
        {
            bw.ReportProgress(0, new ProgressEvent(message, status));
        }

        public void Execute()
        {
            // there's technically a race condition here i think
            // the cost/benefit of fixing it is just not there at the moment
            TParameters p = PrepareParameters();
            if (!InProgress) {
                InProgress = true;
                problems = new List<Problem>();
                statusChanged(Status);
                bw.RunWorkerAsync(p);
            }
        }
    }
}
