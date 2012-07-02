using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Sbt;
using Sbt.CoreTypes;

namespace Core_Wrapper_Test
{
    class Program
    {
        static void Main(string[] args)
        {
            List<ElementInfo> elements = new List<ElementInfo>();

            Point[] points = new Point[4];
            points[0] = new Point(-1, 0, 0);
            points[1] = new Point(0, -1, 0);
            points[2] = new Point(1, 0, 0);
            points[3] = new Point(0, 1, 0);

            Face face = new Face(new Polyloop(points));
            Solid ext = new ExtrudedAreaSolid(face, 0, 0, 1, 1);
            elements.Add(new ElementInfo("foo", ElementType.Slab, 1, ext));

            List<SpaceInfo> spaces = new List<SpaceInfo>();
            ICollection<SpaceBoundary> spaceBoundaries;
            EntryPoint.CalculateSpaceBoundaries(elements, spaces, out spaceBoundaries);
            Console.WriteLine("{0} space boundaries", spaceBoundaries.Count);
        }
    }
}
