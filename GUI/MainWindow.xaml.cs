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
        Dictionary<OperationStatus, ImageSource> overlayImages = new Dictionary<OperationStatus, ImageSource>();

        public MainWindow()
        {
            InitializeComponent();
            overlayImages[OperationStatus.OK] = (ImageSource)FindResource("OKSmall");
            overlayImages[OperationStatus.InProgress] = (ImageSource)FindResource("InProgressSmall");
            overlayImages[OperationStatus.Warnings] = (ImageSource)FindResource("WarningSmall");
            overlayImages[OperationStatus.Errors] = (ImageSource)FindResource("ErrorSmall");
            // the first argument is because binding the output text to a ViewModel property is unusably slow
            // i haven't figured out a better workaround yet
            DataContext = new ViewModel(msg => tbOutput.AppendText(msg), status => SetTaskbarOverlay(status));
        }

        private void MainWindowClosed(object sender, EventArgs e)
        {
            Properties.Settings.Default.Save();
        }

        private void OutputTextChanged(object sender, TextChangedEventArgs e)
        {
            tbOutput.ScrollToEnd();
        }

        private void SetTaskbarOverlay(OperationStatus status)
        {
            ImageSource src;
            overlayImages.TryGetValue(status, out src);
            TaskbarItemInfo.Overlay = src;
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
            var tb = sender as TextBox;
            if (tb != null && filenames.Any())
            {
                tb.Text = filenames[0];
                // this is a workaround for the fact that the update isn't happening automatically
                // this is all code from some internet dude that's helping me anyway
                var expr = tb.GetBindingExpression(TextBox.TextProperty);
                if (expr != null) { expr.UpdateSource(); }
            }
            e.Handled = true; // i don't know what this does but it makes a framework exception go away
        }
    }
}
