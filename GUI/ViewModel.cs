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
                if (PropertyChanged != null)
                {
                    PropertyChanged(this, new PropertyChangedEventArgs("SelectedTabIndex"));
                }
            }
        }

        public string InputIfcFilePath
        {
            get { return Properties.Settings.Default.InputIfcFilename; }
            set
            {
                Properties.Settings.Default.InputIfcFilename = value;
                if (PropertyChanged != null)
                {
                    PropertyChanged(this, new PropertyChangedEventArgs("InputIfcFilePath"));
                }
            }
        }

        public ViewModel()
        {
            BrowseToInputIfcFileCommand = new DelegateCommand((_) => Commands.BrowseToInputIfcFile(this));
        }
    }
}
