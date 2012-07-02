using System;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Linq;
using System.Windows;

namespace GUI_Frontend
{

    struct SBTRunOptions
    {
        public string inputFileName;
        public string outputFileName;
        public bool checkWallColumn;
        public bool checkSlabColumn;
        public bool checkWallSlab;
        public bool verboseBlocking;
        public bool verboseStacking;
        public bool verboseGeometry;
        public bool verboseFenestrations;
        public bool verboseSpaces;
        public bool expensiveChecks;
        public int spaceVerificationTimeout;
        public IntPtr notifyFunc;
        public IntPtr warnFunc;
        public IntPtr errorFunc;
    }

    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
    }
}
