using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using LibIdf.Idd;
using LibIdf.Idf;

using Construction = ConstructionManagement.OutputConstruction;
using Material = ConstructionManagement.OutputLayer;
using Point = Sbt.CoreTypes.Point;

namespace GUI.Operations
{
    partial class IdfGeneration
    {
        class IdfV710Creator : IdfCreator
        {
            private static int ComparePointsForUpperLeftness(Point a, Point b)
            {
                int zcmp = a.Z.CompareTo(b.Z);
                if (zcmp == 0)
                {
                    int xcmp = a.X.CompareTo(b.X) * -1;
                    return xcmp == 0 ? a.Y.CompareTo(b.Y) : xcmp;
                }
                else { return zcmp; }
            }

            private static int ComparePointsForUpperLeftnessFlat(Point a, Point b)
            {
                int ycmp = a.Y.CompareTo(b.Y);
                return ycmp == 0 ? a.X.CompareTo(b.X) * -1 : ycmp;
            }

            private static int ComparePointsForRighterLownessFlat(Point a, Point b)
            {
                int xcmp = a.X.CompareTo(b.X);
                return xcmp == 0 ? a.Y.CompareTo(b.Y) * -1 : xcmp;
            }

            private static int FindFirstCornerIndex(Sbt.CoreTypes.Polyloop loop, Func<Point, Point, int> compare)
            {
                int res = -1;
                for (int i = 0; i < loop.Vertices.Count; ++i)
                {
                    if (res == -1 || compare(loop.Vertices[i], loop.Vertices[res]) > 0)
                    {
                        res = i;
                    }
                }
                return res;
            }

            private static IEnumerable<Point> VertexOrderRotatedGeometry(Sbt.CoreTypes.Polyloop loop, Func<Point, Point, int> comparePoints)
            {
                List<Point> res = new List<Point>();
                int firstIndex = FindFirstCornerIndex(loop, comparePoints);
                int curr = firstIndex;
                do
                {
                    res.Add(loop.Vertices[curr]);
                    curr = (curr + 1) % loop.Vertices.Count;
                }
                while (curr != firstIndex);
                return res;
            }

            public IdfV710Creator(Idf idf, Action<string> notify) : base(idf, notify) { }

            public override void AddBuilding(double northAxis, double loadsConvergence, double tempConvergence, SolarDistribution solarDistribution, BuildingTerrain terrain)
            {
                IdfObject obj = idf.CreateObject("Building");
                obj.Fields["North Axis"].Value = northAxis;
                obj.Fields["Terrain"].Value = terrain.ToString();
                obj.Fields["Loads Convergence Tolerance Value"].Value = loadsConvergence;
                obj.Fields["Temperature Convergence Tolerance Value"].Value = tempConvergence;
                obj.Fields["Solar Distribution"].Value = solarDistribution.ToString();
            }

            public override void AddBuildingSurface(BuildingSurface surf)
            {
                if (surf.Geometry.Vertices.Count > 0)
                {
                    IdfObject obj = 
                        idf.CreateObject("BuildingSurface:Detailed");

                    obj.Fields["Name"].Value = surf.Name;
                    obj.Fields["Zone Name"].Value = surf.ZoneName;
                    obj.Fields["Surface Type"].Value = surf.Type.ToString();

                    obj.Fields["Outside Boundary Condition"].Value = 
                        surf.OtherSideCondition.ToString();
                    if (surf.OtherSideCondition == 
                        BuildingSurface.OtherSideConditionType.Surface)
                    {
                        obj.Fields["Outside Boundary Condition Object"].Value =
                            surf.OtherSideObject;
                    }

                    Func<Point, Point, int> vertexOrderCmp;
                    if (surf.Type == BuildingSurface.SurfaceType.Floor)
                    {
                        vertexOrderCmp = ComparePointsForRighterLownessFlat;
                    }
                    else if (
                        surf.Type == BuildingSurface.SurfaceType.Ceiling ||
                        surf.Type == BuildingSurface.SurfaceType.Roof)
                    {
                        vertexOrderCmp = ComparePointsForUpperLeftnessFlat;
                    }
                    else
                    {
                        vertexOrderCmp = ComparePointsForUpperLeftness;
                    }

                    var rotated =
                        VertexOrderRotatedGeometry(
                            surf.Geometry, 
                            vertexOrderCmp);
                    var withIndices = rotated.Select((p, i) =>
                        new { Point = p, Index = i });

                    foreach (var pair in withIndices)
                    {
                        string name;
                        name = String.Format(
                            "Vertex {0} X-coordinate", 
                            pair.Index + 1);
                        obj.Fields[name].Value = Math.Round(pair.Point.X, 3);
                        name = String.Format(
                            "Vertex {0} Y-coordinate",
                            pair.Index + 1);
                        obj.Fields[name].Value = Math.Round(pair.Point.Y, 3);
                        name = String.Format(
                            "Vertex {0} Z-coordinate",
                            pair.Index + 1);
                        obj.Fields[name].Value = Math.Round(pair.Point.Z, 3);
                    }

                    if (surf.OtherSideCondition != 
                        BuildingSurface.OtherSideConditionType.Outdoors || 
                        surf.Type == 
                        BuildingSurface.SurfaceType.Floor)
                    {
                        obj.Fields["Sun Exposure"].Value = "NoSun";
                        obj.Fields["Wind Exposure"].Value = "NoWind";
                    }

                    obj.Fields["Construction Name"].Value = 
                        surf.Construction.Name;

                    if (!surf.IsVirtual) { 
                        obj.AddComment(
                            "  ! IFC element GUID: " + surf.ElementGuid); 
                    }
                    obj.AddComment(String.Format(
                        "  ! Normal: <{0}, {1}, {2}>", 
                        surf.Normal.X, 
                        surf.Normal.Y, 
                        surf.Normal.Z));
                }
            }

            public override void AddConstantContents()
            {
                IdfObject obj;
                
                idf.CreateObject("Version").Fields["Version Identifier"].Value = "7.1";

                obj = idf.CreateObject("GlobalGeometryRules");
                obj.Fields["Starting Vertex Position"].Value = "UpperLeftCorner";
                obj.Fields["Vertex Entry Direction"].Value = "Counterclockwise";
                obj.Fields["Coordinate System"].Value = "Relative";
                
                idf.CreateObject("Output:Surfaces:Drawing").Fields["Report Type"].Value = "DXF";
                idf.CreateObject("Output:Surfaces:List").Fields["Report Type"].Value = "DetailsWithVertices";
                idf.CreateObject("Output:Surfaces:List").Fields["Report Type"].Value = "ViewFactorInfo";

                obj = idf.CreateObject("Output:Surfaces:List");
                obj.Fields["Report Type"].Value = "Lines";
                obj.Fields["Report Specifications"].Value = "IDF";

                obj = idf.CreateObject("Output:Diagnostics");
                obj.Fields["Key 1"].Value = "DoNotMirrorDetachedShading";
                obj.Fields["Key 2"].Value = "DisplayAdvancedReportVariables";

                idf.CreateObject("Output:Diagnostics").Fields["Key 1"].Value = "DisplayExtraWarnings";
            }

            public override void AddConstruction(Construction c)
            {
                LibIdf.Idf.IdfObject obj = idf.CreateObject("Construction");
                obj.Fields["Name"].Value = c.Name;
                // All constructions *should* have layers, but let's make sure
                // that a bad one doesn't bring down IDF generation entirely.
                if (c.LayerNames.Count > 0)
                {
                    obj.Fields["Outside Layer"].Value = c.LayerNames[0];
                    for (int i = 1; i < c.LayerNames.Count; ++i)
                    {
                        string layerName = String.Format("Layer {0}", i + 1);
                        obj.Fields[layerName].Value = c.LayerNames[i];
                    }
                }
            }

            public override void AddFenestration(FenestrationSurface fenestration)
            {
                if (fenestration.ContainingSurface.OtherSideCondition != BuildingSurface.OtherSideConditionType.Adiabatic && fenestration.Geometry.Vertices.Count > 0)
                {
                    IdfObject obj = idf.CreateObject("FenestrationSurface:Detailed");

                    obj.Fields["Name"].Value = fenestration.Name;
                    obj.Fields["Surface Type"].Value = fenestration.Type.ToString();
                    obj.Fields["Construction Name"].Value = fenestration.Construction.Name;
                    obj.Fields["Building Surface Name"].Value = fenestration.ContainingSurface.Name;
                    obj.Fields["Outside Boundary Condition Object"].Value = fenestration.OtherSideName;
                    foreach (var pair in VertexOrderRotatedGeometry(fenestration.Geometry, ComparePointsForUpperLeftness).Select((point, index) => new { Point = point, Index = index }))
                    {
                        obj.Fields[String.Format("Vertex {0} X-coordinate", pair.Index + 1)].Value = Math.Round(pair.Point.X, 3);
                        obj.Fields[String.Format("Vertex {0} Y-coordinate", pair.Index + 1)].Value = Math.Round(pair.Point.Y, 3);
                        obj.Fields[String.Format("Vertex {0} Z-coordinate", pair.Index + 1)].Value = Math.Round(pair.Point.Z, 3);
                    }
                    obj.AddComment("  ! IFC element GUID: " + fenestration.ElementGuid);
                    obj.AddComment(String.Format("  ! Normal: <{0}, {1}, {2}>", fenestration.Normal.Item1, fenestration.Normal.Item2, fenestration.Normal.Item3));
                }
            }

            public override void AddLocation(string name, double timeZone, double latitude, double longitude, double elevation)
            {
                IdfObject obj = idf.CreateObject("Site:Location");

                obj.Fields["Name"].Value = name;
                obj.Fields["Time Zone"].Value = timeZone;
                obj.Fields["Latitude"].Value = latitude;
                obj.Fields["Longitude"].Value = longitude;
                obj.Fields["Elevation"].Value = elevation;
            }

            public override void AddMaterial(Material layer)
            {
                layer.AddToIdfV710(idf);
            }

            public override void AddRunPeriod(int startMonth, int startDay, int endMonth, int endDay)
            {
                IdfObject obj = idf.CreateObject("RunPeriod");
                obj.Fields["Begin Month"].Value = startMonth;
                obj.Fields["Begin Day of Month"].Value = startDay;
                obj.Fields["End Month"].Value = endMonth;
                obj.Fields["End Day of Month"].Value = endDay;
            }

            public override void AddShading(Shading shading)
            {
                int i = 1;
                foreach (Sbt.CoreTypes.Polyloop face in shading.Faces)
                {
                    IdfObject obj = idf.CreateObject("Shading:Building:Detailed");
                    obj.Fields["Name"].Value = String.Format("{0} (face {1})", shading.SourceName, i);
                    for (int j = 0; j < face.Vertices.Count; ++j)
                    {
                        obj.Fields[String.Format("Vertex {0} X-coordinate", j + 1)].Value = face.Vertices[j].X;
                        obj.Fields[String.Format("Vertex {0} Y-coordinate", j + 1)].Value = face.Vertices[j].Y;
                        obj.Fields[String.Format("Vertex {0} Z-coordinate", j + 1)].Value = face.Vertices[j].Z;
                    }
                    ++i;
                }
            }

            public override void AddTimestep(int timestep)
            {
                idf.CreateObject("Timestep").Fields["Number of Timesteps per Hour"].Value = timestep;
            }

            public override void AddZone(string name)
            {
                IdfObject obj = idf.CreateObject("Zone");
                obj.Fields["Name"].Value = name;
            }
        }
    }
}
