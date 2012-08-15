using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using IdfConstruction = GUI.Constructions.MaterialLayer;

namespace GUI.Operations
{
    static partial class IdfGeneration
    {
        class Parameters
        {
            public string OutputFilename { get; set; }
            public SbtBuildingInformation SbtBuilding { get; set; }
            public IfcInformationExtractor.BuildingInformation IfcBuilding { get; set; }
            public IDictionary<string, IfcConstruction> IfcConstructionsByName { get; set; }
            public EnergyPlusVersion EPVersion { get; set; }
            public Func<LibIdf.Idd.Idd> GetIdd { get; set; }
            public Action<string> Notify { get; set; }
        }
    }
}
