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
            DataContext = new ViewModel();
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
    }
}
