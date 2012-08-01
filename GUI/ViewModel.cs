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
        private string _inputIfcPath;
        private string _outputIfcPath;

        public event PropertyChangedEventHandler PropertyChanged;

        public ICommand BrowseToInputIfcFileCommand { get; private set; }

        public string InputIfcFilePath
        {
            get { return _inputIfcPath; }
            set
            {
                _inputIfcPath = value;
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
