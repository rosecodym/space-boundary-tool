using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace GUI.Operations
{
    static partial class IdfGeneration
    {
        public enum SolarDistribution
        {
            MinimalShadowing,
            FullExterior,
            FullExteriorWithReflections,
            FullInteriorAndExterior,
            FullInteriorAndExteriorWithReflections
        }

        public enum BuildingTerrain
        {
            Country,
            Suburbs,
            City,
            Ocean,
            Urban
        }

        class Parameters
        {
            public string OutputFilename { get; set; }
            public string LocationName { get; set; }
            public double TimeZone { get; set; }
            public SolarDistribution SolarDistribution { get; set; }
            public double NorthAxis { get; set; }
            public BuildingTerrain BuildingTerrain { get; set; }
            public double LoadsConvergenceTolerance { get; set; }
            public double TemperatureConvergenceTolerance { get; set; }
            public int StartMonth { get; set; }
            public int StartDay { get; set; }
            public int EndMonth { get; set; }
            public int EndDay { get; set; }
            public int Timestep { get; set; }
            public SbtBuildingInformation SbtBuilding { get; set; }
            public IfcInformationExtractor.BuildingInformation IfcBuilding { get; set; }
            public IDictionary<string, IfcConstruction> IfcConstructionsByName { get; set; }
            public EnergyPlusVersion EPVersion { get; set; }
            public Func<LibIdf.Idd.Idd> GetIdd { get; set; }
            public Action<string> Notify { get; set; }
        }
    }
}
