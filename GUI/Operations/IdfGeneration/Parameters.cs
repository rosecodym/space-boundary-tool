﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace GUI.Operations
{
    static partial class IdfGeneration
    {
        class Parameters
        {
            public string OutputFilename { get; set; }
            public SbtBuildingInformation Building { get; set; }
            public EnergyPlusVersion EPVersion { get; set; }
            public Func<LibIdf.Idd.Idd> GetIdd { get; set; }
            public Action<string> Notify { get; set; }
        }
    }
}