using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using LibIdf.Idd;
using LibIdf.Idf;

using Point = Sbt.CoreTypes.Point;

namespace GUI.Operations
{
    static partial class IdfGeneration
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

            public override void AddBuildingSurface(BuildingSurface surf)
            {
                if (surf.IsLargeEnoughForWriting(0.01))
                {
                    IdfObject obj = idf.CreateObject("BuildingSurface:Detailed");

                    obj.Fields["Name"].Value = surf.Name;
                    obj.Fields["Zone Name"].Value = surf.ZoneName;
                    obj.Fields["Surface Type"].Value = surf.Type.ToString();

                    if (surf.IsConnectedToGround) { obj.Fields["Outside Boundary Condition"].Value = "Ground"; }
                    else if (surf.IsExternal) { obj.Fields["Outside Boundary Condition"].Value = "Outdoors"; }
                    else if (surf.OtherSideName != null)
                    {
                        obj.Fields["Outside Boundary Condition"].Value = "Surface";
                        obj.Fields["Outside Boundary Condition Object"].Value = surf.OtherSideName;
                    }
                    else { obj.Fields["Outside Boundary Condition"].Value = "Adiabatic"; }

                    Func<Point, Point, int> vertexOrderComparer =
                        surf.Type == BuildingSurface.SurfaceType.Floor ? ComparePointsForRighterLownessFlat :
                        surf.Type == BuildingSurface.SurfaceType.Ceiling ? ComparePointsForUpperLeftnessFlat :
                        surf.Type == BuildingSurface.SurfaceType.Roof ? (Func<Point, Point, int>)ComparePointsForUpperLeftnessFlat : ComparePointsForUpperLeftness;

                    foreach (var pair in VertexOrderRotatedGeometry(surf.Geometry, vertexOrderComparer).Select((point, index) => new { Point = point, Index = index }))
                    {
                        obj.Fields[String.Format("Vertex {0} X-coordinate", pair.Index + 1)].Value = pair.Point.X;
                        obj.Fields[String.Format("Vertex {0} Y-coordinate", pair.Index + 1)].Value = pair.Point.Y;
                        obj.Fields[String.Format("Vertex {0} Z-coordinate", pair.Index + 1)].Value = pair.Point.Z;
                    }

                    if (!surf.IsExternal || surf.Type == BuildingSurface.SurfaceType.Floor)
                    {
                        obj.Fields["Sun Exposure"].Value = "NoSun";
                        obj.Fields["Wind Exposure"].Value = "NoWind";
                    }

                    obj.Fields["Construction Name"].Value = surf.ConstructionName;

                    if (!surf.IsVirtual) { obj.AddComment("  ! IFC element GUID: " + surf.ElementGuid); }
                    obj.AddComment(String.Format("  ! Normal: <{0}, {1}, {2}>", surf.Normal.Item1, surf.Normal.Item2, surf.Normal.Item3));
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

            public override void AddConstruction(Materials.Output.Construction c)
            {
                LibIdf.Idf.IdfObject obj = idf.CreateObject("Construction");
                obj.Fields["Name"].Value = c.Name;
                obj.Fields["Outside Layer"].Value = c.LayerNames[0];
                for (int i = 1; i < c.LayerNames.Count; ++i)
                {
                    obj.Fields[String.Format("Layer {0}", i + 1)].Value = c.LayerNames[i];
                }
            }

            public override void AddFenestration(FenestrationSurface fenestration)
            {
                if (fenestration.IsLargeEnoughForWriting(0.01))
                {
                    IdfObject obj = idf.CreateObject("FenestrationSurface:Detailed");

                    obj.Fields["Name"].Value = fenestration.Name;
                    obj.Fields["Surface Type"].Value = fenestration.Type.ToString();
                    obj.Fields["Construction Name"].Value = fenestration.ConstructionName;
                    obj.Fields["Building Surface Name"].Value = fenestration.ContainingSurfaceName;
                    obj.Fields["Outside Boundary Condition Object"].Value = fenestration.OtherSideName;
                    foreach (var pair in VertexOrderRotatedGeometry(fenestration.Geometry, ComparePointsForUpperLeftness).Select((point, index) => new { Point = point, Index = index }))
                    {
                        obj.Fields[String.Format("Vertex {0} X-coordinate", pair.Index + 1)].Value = pair.Point.X;
                        obj.Fields[String.Format("Vertex {0} Y-coordinate", pair.Index + 1)].Value = pair.Point.Y;
                        obj.Fields[String.Format("Vertex {0} Z-coordinate", pair.Index + 1)].Value = pair.Point.Z;
                    }
                }
            }

            public override void AddMaterial(Materials.Output.MaterialLayer layer)
            {
                layer.AddToIdfV710(idf);
            }

            public override void AddZone(string name, string sourceGuid)
            {
                LibIdf.Idf.IdfObject obj = idf.CreateObject("Zone");
                obj.Fields["Name"].Value = name;
                obj.AddComment("! space GUID is " + sourceGuid);
            }
        }
    }
}
