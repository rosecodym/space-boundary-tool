using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace GUI.Operations
{
    static partial class IdfGeneration
    {
        class Shading
        {
            string sourceName;
            List<Sbt.CoreTypes.Face> faces;

            public Shading(string sourceName, Sbt.CoreTypes.Solid baseGeometry, IEnumerable<BuildingSurface> externalWallBoundaries)
            {
                this.sourceName = sourceName;
                faces = new List<Sbt.CoreTypes.Face>(baseGeometry.ToFaces());
            }

            public string SourceName { get { return sourceName; } }

            public IEnumerable<Sbt.CoreTypes.Polyloop> Faces
            {
                get
                {
                    return faces.Select(f => f.OuterBoundary);
                }
            }
        }
    }
}
