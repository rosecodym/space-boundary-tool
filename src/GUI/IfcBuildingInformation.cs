using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using ConstructionManagement.ModelConstructions;
using IfcInterface;

namespace GUI
{
    // If this class seems a little weird, it's because it's a facade.
    class IfcBuildingInformation
    {
        public IfcBuildingInformation(string path)
        {
            throw new NotImplementedException();
        }

        public String Filename { get; private set; }
        public double Elevation { get; private set; }
        public double NorthAxis { get; private set; }
        public double Latitude { get; private set; }
        public double Longitude { get; private set; }

        public IDictionary<string, IfcSpace> SpacesByGuid { get; private set; }
        public IDictionary<string, IfcElement> ElementsByGuid { get; private set; }
        public ICollection<ModelMappingSource> ConstructionMappingSources
        {
            get;
            private set;
        }
    }
}
