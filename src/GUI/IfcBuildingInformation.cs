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
        ModelConstructionCollection cs;
        Dictionary<string, ModelConstruction> constructionsByElementGuid =
            new Dictionary<string, ModelConstruction>();

        public IfcBuildingInformation(string path)
        {
            using (var model = new IfcModel(path))
            {
                Filename = path;
                Elevation = model.Elevation;
                Elements = model.Elements;
                NorthAxis = model.NorthAxis;
                Latitude = model.Latitude;
                Longitude = model.Longitude;
                SpacesByGuid = new Dictionary<string, IfcSpace>();
                foreach (var space in model.Spaces)
                {
                    SpacesByGuid[space.Guid] = space;
                }
                double mplu = 1 / model.LengthUnitsPerMeter;
                cs = new ModelConstructionCollection(mplu);
                foreach (var element in Elements)
                {
                    if (element.LayerNames == null)
                    {
                        string name =
                            "(construction for missing material properties)";
                        var c = cs.GetModelConstructionSingleOpaque(name);
                        constructionsByElementGuid[element.Guid] = c;
                    }
                    else if (element.IsWindow)
                    {
                        string name = element.LayerNames[0];
                        var c = cs.GetModelConstructionWindow(name);
                        constructionsByElementGuid[element.Guid] = c;
                    }
                    else if (element.LayerNames.Count == 1)
                    {
                        // A single-layer composite will have its thickness
                        // dropped. I'm not sure why but it hasn't caused any
                        // problems so far so I'm not going to change it.
                        string name = element.LayerNames[0];
                        var c = cs.GetModelConstructionSingleOpaque(name);
                        constructionsByElementGuid[element.Guid] = c;
                    }
                    else
                    {
                        string cname = element.ConstructionCompositeName;
                        var ns = element.LayerNames;
                        var ts = element.LayerThicknesses;
                        var c = cs.GetModelConstructionLayerSet(cname, ns, ts);
                        constructionsByElementGuid[element.Guid] = c;
                    }
                }
            }
        }

        public String Filename { get; private set; }
        public ICollection<IfcElement> Elements { get; private set; }
        public double Elevation { get; private set; }
        public double NorthAxis { get; private set; }
        public double Latitude { get; private set; }
        public double Longitude { get; private set; }

        public IDictionary<string, IfcSpace> SpacesByGuid { get; private set; }
        public IEnumerable<ModelMappingSource> ConstructionMappingSources
        {
            get { return cs.MappingSources; }
        }

        public ModelConstruction ConstructionForElementGuid(String guid)
        {
            ModelConstruction res;
            if (!constructionsByElementGuid.TryGetValue(guid, out res))
            {
                return null;
            }
            return res;
        }
    }
}
