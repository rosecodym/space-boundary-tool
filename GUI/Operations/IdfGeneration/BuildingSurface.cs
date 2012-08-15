using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using Sbt.CoreTypes;

namespace GUI.Operations
{
    static partial class IdfGeneration
    {
        class BuildingSurface
        {
            public enum SurfaceType
            {
                Floor,
                Wall,
                Ceiling,
                Roof
            }

            readonly SpaceBoundary sbtInfo;
            readonly string constructionName;
            readonly string zoneName;
            readonly bool connectedToGround;

            public BuildingSurface(SpaceBoundary sb, string constructionName, string zoneName, bool connectedToGround)
            {
                this.sbtInfo = sb;
                this.constructionName = constructionName;
                this.zoneName = zoneName;
                this.connectedToGround = connectedToGround;
            }

            public bool IsLargeEnoughForWriting(double epsilon)
            {
                Func<Point, Point, double> distance =
                (a, b) => Math.Sqrt((a.X - b.X) * (a.X - b.X) + (a.Y - b.Y) * (a.Y - b.Y) + (a.Z - b.Z) * (a.Z - b.Z));

                var vertices = Geometry.Vertices;

                int tooCloseCount = 0;
                for (int i = 0; i < vertices.Count; ++i)
                {
                    if (distance(vertices[i], vertices[(i + 1) % vertices.Count]) < epsilon)
                    {
                        ++tooCloseCount;
                    }
                }

                return tooCloseCount < vertices.Count - 2;
            }

            public string Name { get { return sbtInfo.Guid; } }
            public string ZoneName { get { return zoneName; } }
            public string ConstructionName { get { return constructionName; } }
            public SurfaceType Type
            {
                get
                {
                    if (Normal.Item1 != 0 || Normal.Item2 != 0) { return SurfaceType.Wall; }
                    else if (Normal.Item3 < 0) { return SurfaceType.Floor; }
                    else { return IsExternal ? SurfaceType.Roof : SurfaceType.Ceiling; }
                }
            }
            public bool IsConnectedToGround { get { return connectedToGround; } }
            public bool IsExternal { get { return sbtInfo.IsExternal; } }
            public bool IsVirtual { get { return sbtInfo.IsVirtual; } }
            public string ElementGuid { get { return sbtInfo.Element != null ? sbtInfo.Element.Guid : null; } }
            public string OtherSideName
            {
                get
                {
                    return sbtInfo.Opposite != null ? sbtInfo.Opposite.Guid : null;
                }
            }
            public Sbt.CoreTypes.Polyloop Geometry { get { return sbtInfo.Geometry; } }
            public Tuple<double, double, double> Normal { get { return sbtInfo.Normal; } }
        }
    }
}
