using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using Sbt.CoreTypes;

using Construction = ConstructionManagement.OutputConstructionBase;

namespace GUI.Operations
{
    partial class IdfGeneration
    {
        class FenestrationSurface
        {
            const double VertexAdjustmentAmount = 0.01;

            public enum FenestrationType
            {
                Door,
                Window
            }

            readonly SpaceBoundary sbtInfo;
            readonly BuildingSurface containingSurface;
            readonly Construction construction;

            public FenestrationSurface(
                SpaceBoundary sbtInfo, 
                BuildingSurface containingSurface, 
                Construction construction)
            {
                this.sbtInfo = sbtInfo;
                this.containingSurface = containingSurface;
                this.construction = construction;
            }

            public string Name { get { return sbtInfo.Guid; } }
            public FenestrationType Type { get { return sbtInfo.Element.Type == ElementType.Window ? FenestrationType.Window : FenestrationType.Door; } }
            public Construction Construction { get { return construction; } }
            public BuildingSurface ContainingSurface { get { return containingSurface; } }
            public string OtherSideName { get { return sbtInfo.Opposite != null ? sbtInfo.Opposite.Guid : null; } }
            public Polyloop Geometry { get { return PullVerticesTowardCenter(sbtInfo.Geometry).Cleaned(0.01); } }
            public string ElementGuid { get { return sbtInfo.Element.Guid; } }
            public Tuple<double, double, double> Normal { get { return sbtInfo.Normal; } }

            private Polyloop PullVerticesTowardCenter(Polyloop original)
            {
                int pcount = original.Vertices.Count;
                var coordinateSum = original.Vertices.Aggregate(
                    new { X = 0.0, Y = 0.0, Z = 0.0 },
                    (accum, p) => new { X = accum.X + p.X, Y = accum.Y + p.Y, Z = accum.Z + p.Z });
                var center = new { X = coordinateSum.X / pcount, Y = coordinateSum.Y / pcount, Z = coordinateSum.Z / pcount };
                var modifiedCoordinates = original.Vertices.Select(p =>
                {
                    var diff = new { Dx = center.X - p.X, Dy = center.Y - p.Y, Dz = center.Z - p.Z };
                    double magnitude = Math.Sqrt(diff.Dx * diff.Dx + diff.Dy * diff.Dy + diff.Dz * diff.Dz);
                    var normalized = new { Dx = diff.Dx / magnitude, Dy = diff.Dy / magnitude, Dz = diff.Dz / magnitude };
                    var delta = new { Dx = normalized.Dx * VertexAdjustmentAmount, Dy = normalized.Dy * VertexAdjustmentAmount, Dz = normalized.Dz * VertexAdjustmentAmount };
                    return new Point(p.X + delta.Dx, p.Y + delta.Dy, p.Z + delta.Dz);
                });
                return new Polyloop(modifiedCoordinates);
            }
        }
    }
}
