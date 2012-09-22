using System;
using System.Collections.Generic;
using System.Linq;
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

namespace GUI
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
            // this argument is because binding the output text to a ViewModel property is unusably slow
            // i haven't figured out a better workaround yet
            DataContext = new ViewModel(msg => tbOutput.AppendText(msg));
        }

        private void MainWindowClosed(object sender, EventArgs e)
        {
            // "no code in the code-behind" but it's just this one thing i swear
            Properties.Settings.Default.Save();
        }

        private void OutputTextChanged(object sender, TextChangedEventArgs e)
        {
            // whoops guess i was wrong about "one thing"
            tbOutput.ScrollToEnd();
        }

        private static IList<string> GetFileNames(IDataObject dataObject)
        {
            if (dataObject != null && dataObject.GetDataPresent("FileNameW"))
            {
                var filenames = dataObject.GetData("FileNameW") as string[];
                return filenames ?? new string[0];
            }
            return new string[0];
        }

        private void TextBox_PreviewDrag(object sender, DragEventArgs e)
        {
            if (GetFileNames(e.Data).Any())
            {
                e.Effects = DragDropEffects.All;
                e.Handled = true;
            }
        }

        private void TextBox_PreviewDrop(object sender, DragEventArgs e)
        {
            var filenames = GetFileNames(e.Data);
            if (filenames.Any())
            {
                ((TextBox)sender).Text = filenames[0];
            }
        }
    }
}
