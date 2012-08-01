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

        public string InputIfcFilePath
        {
            get { return Properties.Settings.Default.inputIfcFilename; }
            set
            {
                Properties.Settings.Default.inputIfcFilename = value;
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
