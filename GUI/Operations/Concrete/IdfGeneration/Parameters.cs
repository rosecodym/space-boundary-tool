using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using ModelConstruction = ConstructionManagement.ModelConstructions.ModelConstruction;
using Normal = System.Tuple<double, double, double>;

namespace GUI.Operations
{
    partial class IdfGeneration
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

        public class Parameters
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
            public Func<int, ModelConstruction> MaterialIDToModelConstruction { get; set; }
            public Func<int, Normal> MaterialIDToCompositeDir { get; set; }
            public EnergyPlusVersion EPVersion { get; set; }
            public Func<LibIdf.Idd.Idd> GetIdd { get; set; }
            public bool AttachDebugger { get; set; }
        }
    }
}
