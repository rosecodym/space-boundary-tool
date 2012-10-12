using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace GUI.Operations
{
    partial class IdfGeneration
    {
        class Shading
        {
            static double Distance(Sbt.CoreTypes.Point a, Sbt.CoreTypes.Point b)
            {
                return Math.Sqrt((a.X - b.X) * (a.X - b.X) + (a.Y - b.Y) * (a.Y - b.Y) + (a.Z - b.Z) * (a.Z - b.Z));
            }

            static Tuple<double, double, double> CalculateTranslation(double dx, double dy, double dz, double length)
            {
                double mag = Math.Sqrt(dx * dx + dy * dy + dz * dz);
                return Tuple.Create(dx / mag * length, dy / mag * length, dz / mag * length);
            }

            string sourceName;
            List<Sbt.CoreTypes.Polyloop> faces;

            public Shading(string sourceName, Sbt.CoreTypes.Solid baseGeometry, IEnumerable<BuildingSurface> externalWallBoundaries)
            {
                this.sourceName = sourceName;
                faces = new List<Sbt.CoreTypes.Polyloop>(baseGeometry.ToFaces().Select(f => f.OuterBoundary));

                Sbt.CoreTypes.Point shadingCenter = baseGeometry.AverageOfFaceCenters();

                BuildingSurface closestBoundary = null;
                double currMinDistance = double.PositiveInfinity;
                foreach (BuildingSurface w in externalWallBoundaries)
                {
                    double distance = Distance(w.Geometry.Center(), shadingCenter);
                    if (distance < currMinDistance)
                    {
                        currMinDistance = distance;
                        closestBoundary = w;
                    }
                }

                int closestFaceIndex = -1;
                currMinDistance = double.PositiveInfinity;
                Sbt.CoreTypes.Point boundaryCenter = closestBoundary.Geometry.Center();
                for (int i = 0; i < faces.Count; ++i)
                {
                    double distance = Distance(boundaryCenter, faces[i].Center());
                    if (distance < currMinDistance)
                    {
                        currMinDistance = distance;
                        closestFaceIndex = i;
                    }
                }

                Tuple<double, double, double> translation = CalculateTranslation(
                    -closestBoundary.Normal.Item1,
                    -closestBoundary.Normal.Item2,
                    -closestBoundary.Normal.Item3,
                    currMinDistance);

                faces.RemoveAt(closestFaceIndex);

                faces = new List<Sbt.CoreTypes.Polyloop>(faces.Select(f => f.Translate(translation.Item1, translation.Item2, translation.Item3)));
            }

            public string SourceName { get { return sourceName; } }

            public IEnumerable<Sbt.CoreTypes.Polyloop> Faces
            {
                get
                {
                    return faces;
                }
            }
        }
    }
}
