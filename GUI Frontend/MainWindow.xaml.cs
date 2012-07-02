using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace GUI_Frontend
{

    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        [UnmanagedFunctionPointer(System.Runtime.InteropServices.CallingConvention.Cdecl)]
        private delegate void MessageDelegate(string msg);

        List<SBSummary> spaceBoundaryCounts;

        private BackgroundWorker worker = new BackgroundWorker();
        MessageDelegate messageDelegate;

        public List<SBSummary> SpaceBoundaryCounts { get { return spaceBoundaryCounts; } }

        private void GotMessage(string msg)
        {
            worker.ReportProgress(0, msg);
        }

        public MainWindow()
        {
            List<SBSummary> spaceBoundaryCounts = new List<SBSummary>();
            InitializeComponent();
            messageDelegate = new MessageDelegate(GotMessage);
            worker.WorkerReportsProgress = true;
            worker.DoWork += new DoWorkEventHandler(worker_DoWork);
            worker.ProgressChanged += new ProgressChangedEventHandler(worker_ProgressChanged);
            worker.RunWorkerCompleted += new RunWorkerCompletedEventHandler(worker_RunWorkerCompleted);
        }

        void worker_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            runButton.IsEnabled = true;
            //Nullable<SBTInterface.SBCounts> result = (Nullable<SBTInterface.SBCounts>)e.Result;
            //if (result != null)
            //{
            //    spaceBoundaryCounts = SBSummary.GetAllSpaceSummaries(result.Value);
            //    tabSBSummaries.IsEnabled = true;
            //}
        }

        void worker_DoWork(object sender, DoWorkEventArgs e)
        {
            SBTRunOptions opts = (SBTRunOptions)e.Argument;
            Nullable<SBTInterface.SBCounts> result = SBTInterface.InvokeSBT(opts, worker);
            e.Result = result;
        }

        void worker_ProgressChanged(object sender, ProgressChangedEventArgs e)
        {
            tbOutput.AppendText((string)e.UserState);
            tbOutput.ScrollToEnd();
        }

        private void inputFileBrowseButton_Click(object sender, RoutedEventArgs e)
        {
            OpenFileDialog dlg = new OpenFileDialog();
            dlg.CheckFileExists = true;
            dlg.CheckPathExists = true;
            dlg.DereferenceLinks = true;
            dlg.Filter = "IFC files|*.ifc";
            dlg.Title = "Select input IFC file";

            Nullable<bool> res = dlg.ShowDialog();
            if (res == true)
            {
                Properties.Settings.Default.inputFileName = dlg.FileName;
                Properties.Settings.Default.outputFileName = dlg.FileName;
            }
        }

        private void outputFileBrowseButton_Click(object sender, RoutedEventArgs e)
        {
            SaveFileDialog dlg = new SaveFileDialog();
            dlg.Filter = "IFC files|*.ifc";
            dlg.Title = "Select output IFC file";

            Nullable<bool> res = dlg.ShowDialog();
            if (res == true)
            {
                Properties.Settings.Default.outputFileName = dlg.FileName;
            }
        }

        private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            Properties.Settings.Default.Save();
        }

        private void button3_Click(object sender, RoutedEventArgs e)
        {
            runButton.IsEnabled = false;
            tabs.SelectedIndex = 1;
            worker.RunWorkerAsync(CreateOptions());
        }

        private SBTRunOptions CreateOptions()
        {
            SBTRunOptions opts;
            opts.inputFileName = tbInputFileName.Text;
            opts.outputFileName = tbOutputFileName.Text;
            opts.checkSlabColumn = (bool)cbCheckSlabColumn.IsChecked;
            opts.checkWallColumn = (bool)cbCheckWallColumn.IsChecked;
            opts.checkWallSlab = (bool)cbCheckWallSlab.IsChecked;
            opts.expensiveChecks = (bool)cbExpensiveValidation.IsChecked;
            opts.verboseBlocking = (bool)cbVerboseBlocks.IsChecked;
            opts.verboseStacking = (bool)cbVerboseStacks.IsChecked;
            opts.verboseGeometry = (bool)cbVerboseGeometry.IsChecked;
            opts.verboseFenestrations = (bool)cbVerboseFenestrations.IsChecked;
            opts.verboseSpaces = (bool)cbVerboseSpaces.IsChecked;
            opts.spaceVerificationTimeout = (bool)cbTimeoutSpaceVerification.IsChecked ? System.Int32.Parse(tbSpaceVerificationTimeout.Text) : -1;
            opts.notifyFunc = Marshal.GetFunctionPointerForDelegate(messageDelegate);
            opts.warnFunc = Marshal.GetFunctionPointerForDelegate(messageDelegate);
            opts.errorFunc = Marshal.GetFunctionPointerForDelegate(messageDelegate);
            return opts;
        }

    }
}
