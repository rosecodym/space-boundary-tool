using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using Sbt.CoreTypes;
using Vector3 = GUI.SbtExtensions.Vector3;

namespace GUI.Operations
{
    partial class IdfGeneration
    {
        class Shading
        {
            public class ShadingConstructionFailedException : Exception
            {
                public ShadingConstructionFailedException(string msg) :
                    base(msg) { }
            }

            readonly string sourceName;
            readonly List<Polyloop> faces;

            public Shading(
                string sourceName, 
                Solid baseGeometry, 
                IEnumerable<BuildingSurface> externalWallBoundaries)
            {
                this.sourceName = sourceName;
                this.faces = new List<Polyloop>(
                    baseGeometry.ToFaces().Select(f => f.OuterBoundary));

                // I think this algorithm will "fail" in certain pathological
                // cases, but I don't think E+ will notice. Once again, there's
                // no point in being smarter than E+.

                Vector3[] fNorms = 
                    faces.Select(loop => loop.Plane().Normal).ToArray();

                BuildingSurface closestWall;
                int closestFaceIx = -1;
                // the starting value is the maximum distance a shading will be
                // adjusted, in meters
                double closestDist = 0.5;
                foreach (BuildingSurface w in externalWallBoundaries) {
                    for (int i = 0; i < faces.Count; ++i)
                    {
                        if (Vector3.AreAntiparallel(w.Normal, fNorms[i]))
                        {
                            double d = Distance(faces[i], w);
                            // a negative distance means the shading is
                            // "inside" this wall boundary, which means that
                            // it's the wrong axis. so we ignore it.
                            if (d >= 0.0 && d < closestDist)
                            {
                                closestWall = w;
                                closestFaceIx = i;
                                closestDist = d;
                            }
                        }
                    }
                }

                if (closestFaceIx == -1)
                {
                    throw new ShadingConstructionFailedException(
                        String.Format(
                            "Shading {0} could not be adjusted to the " +
                                "building facade. This usually means that " +
                                "its geometry is too unusual or it has been " +
                                "incorrectly placed.",
                            sourceName));
                }

                Vector3 translation = 
                    CalculateTranslation(fNorms[closestFaceIx], closestDist);
                List<Polyloop> newFaces = new List<Polyloop>();
                for (int i = 0; i < faces.Count; ++i)
                {
                    if (i != closestFaceIx)
                    {
                        newFaces.Add(faces[i].Translate(translation));
                    }
                }
                faces = newFaces;
            }

            public string SourceName { get { return sourceName; } }

            public IEnumerable<Sbt.CoreTypes.Polyloop> Faces
            {
                get
                {
                    return faces;
                }
            }


            static double Distance(Point a, Point b)
            {
                return Math.Sqrt(
                    (a.X - b.X) * (a.X - b.X) +
                    (a.Y - b.Y) * (a.Y - b.Y) +
                    (a.Z - b.Z) * (a.Z - b.Z));
            }

            static double Distance(Polyloop face, BuildingSurface surf)
            {
                // precondition: parallel
                return surf.Plane.SignedDistanceFrom(face.Vertices[0]);
            }

            static Vector3 CalculateTranslation(Vector3 d, double len)
            {
                double mag = Math.Sqrt(d.X * d.X + d.Y * d.Y + d.Z * d.Z);
                return new Vector3(
                    d.X / mag * len,
                    d.Y / mag * len,
                    d.Z / mag * len);
            }
        }
    }
}
