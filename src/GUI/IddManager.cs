using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;

using IdfToolbox.Idd;

namespace GUI
{
    class IddManager
    {
        private Dictionary<EnergyPlusVersion, Idd> loaded = new Dictionary<EnergyPlusVersion, Idd>();

        private Dictionary<EnergyPlusVersion, string> resources = new Dictionary<EnergyPlusVersion, string>()
        {
            { EnergyPlusVersion.V710, Properties.Resources.V7_1_0_Energy_ },
            { EnergyPlusVersion.V720, Properties.Resources.V7_2_0_Energy_ },
            { EnergyPlusVersion.V720C, Properties.Resources.V7_2_0C_Energy_ },
            { EnergyPlusVersion.V800, Properties.Resources.V8_0_0_Energy_ }
        };

        public Idd GetIddFor(EnergyPlusVersion version, Action<string> warn)
        {
            if (!loaded.ContainsKey(version))
            {
                LoadIddFor(version, warn);
            }
            return loaded[version];
        }

        private void LoadIddFor(EnergyPlusVersion version, Action<string> warn)
        {
            string filename = Path.GetTempFileName();
            using (StreamWriter writer = new StreamWriter(filename))
            {
                writer.Write(resources[version]);
            }
            loaded[version] = new Idd(filename);
            foreach (var w in loaded[version].Warnings) { warn(w); }
            File.Delete(filename);
        }

        static public EnergyPlusVersion StringToVersion(
            string str, 
            EnergyPlusVersion def = EnergyPlusVersion.V800)
        {
            if (str == "7.1") { return EnergyPlusVersion.V710; }
            else if (str == "7.2") { return EnergyPlusVersion.V720; }
            else if (str == "8.0") { return EnergyPlusVersion.V800; }
            else { return def; }
        }
    }
}
