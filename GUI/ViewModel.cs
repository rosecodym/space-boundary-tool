using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Windows.Input;

namespace GUI
{
    class ViewModel : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        public ICommand BrowseToInputIfcFileCommand { get; private set; }

        public int SelectedTabIndex
        {
            get { return Properties.Settings.Default.SelectedTabIndex; }
            set
            {
                Properties.Settings.Default.SelectedTabIndex = value;
                if (PropertyChanged != null) { PropertyChanged(this, new PropertyChangedEventArgs("SelectedTabIndex")); }
            }
        }

        public string InputIfcFilePath
        {
            get { return Properties.Settings.Default.InputIfcFilename; }
            set
            {
                Properties.Settings.Default.InputIfcFilename = value;
                if (PropertyChanged != null) { PropertyChanged(this, new PropertyChangedEventArgs("InputIfcFilePath")); }
            }
        }

        public string OutputIfcFilePath
        {
            get { return Properties.Settings.Default.OutputIfcFilename; }
            set
            {
                Properties.Settings.Default.OutputIfcFilename = value;
                if (PropertyChanged != null) { PropertyChanged(this, new PropertyChangedEventArgs("OutputIfcFilePath")); }
            }
        }

        public bool SkipWallColumnCheck
        {
            get { return Properties.Settings.Default.SkipWallColumnCheck; }
            set
            {
                Properties.Settings.Default.SkipWallColumnCheck = value;
                if (PropertyChanged != null) { PropertyChanged(this, new PropertyChangedEventArgs("SkipWallColumnCheck")); }
            }
        }

        public bool SkipSlabColumnCheck
        {
            get { return Properties.Settings.Default.SkipSlabColumnCheck; }
            set
            {
                Properties.Settings.Default.SkipSlabColumnCheck = value;
                if (PropertyChanged != null) { PropertyChanged(this, new PropertyChangedEventArgs("SkipSlabColumnCheck")); }
            }
        }

        public bool SkipWallSlabCheck
        {
            get { return Properties.Settings.Default.SkipWallSlabCheck; }
            set
            {
                Properties.Settings.Default.SkipWallSlabCheck = value;
                if (PropertyChanged != null) { PropertyChanged(this, new PropertyChangedEventArgs("SkipWallSlabCheck")); }
            }
        }

        public bool WriteIfc
        {
            get { return Properties.Settings.Default.CreateOutputIfc; }
            set
            {
                Properties.Settings.Default.CreateOutputIfc = value;
                if (PropertyChanged != null) { PropertyChanged(this, new PropertyChangedEventArgs("WriteIfc")); }
            }
        }

        public ViewModel()
        {
            BrowseToInputIfcFileCommand = new DelegateCommand((_) => Commands.BrowseToInputIfcFile(this));
        }
    }
}
