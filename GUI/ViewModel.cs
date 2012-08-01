using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Windows.Input;

using Microsoft.Win32;

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
            BrowseToInputIfcFileCommand = new DelegateCommand((_) => BrowseToInputIfcFile());
        }

        private void BrowseToInputIfcFile()
        {
            OpenFileDialog ofd = new OpenFileDialog();
            bool? result = ofd.ShowDialog();
            if (result.HasValue && result.Value == true)
            {
                InputIfcFilePath = ofd.FileName;
            }
        }

        /*
         *  

        public ViewModel()
        {
            BrowseCommand = new DelegateCommand((o) => BrowseToFile());
        }

        private void BrowseToFile()
        {
            var openFileDialog = new OpenFileDialog();
            var result = openFileDialog.ShowDialog();

            if (result.HasValue && result.Value == true)
            {
                FilePath = openFileDialog.FileName;
            }
        }*/
    }
}
