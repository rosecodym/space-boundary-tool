using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using LibIdf.Idd;

namespace GUI
{
    class IddManager
    {
        private Dictionary<EnergyPlusVersion, Idd> loaded;

        Dictionary<EnergyPlusVersion, string> resources = new Dictionary<EnergyPlusVersion, string>()
        {
            { EnergyPlusVersion.V710, Properties.Resources.V7_1_0_Energy_ }
        };
    }
}
