using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Sbt.CoreTypes
{
    public class ExtrudedAreaSolid : Solid
    {
        private readonly Face area;
        private readonly double dx;
        private readonly double dy;
        private readonly double dz;
        private readonly double depth;

        public Face Area { get { return area; } }
        public double Dx { get { return dx; } }
        public double Dy { get { return dy; } }
        public double Dz { get { return dz; } }
        public double Depth { get { return depth; } }

        internal ExtrudedAreaSolid(NativeCoreTypes.ExtrudedAreaSolid native)
            : this(
                new Face(native.area),
                native.dx,
                native.dy,
                native.dz,
                native.depth)
        { }

        public ExtrudedAreaSolid(Face area, double dx, double dy, double dz, double depth)
        {
            this.area = area;
            this.dx = dx;
            this.dy = dy;
            this.dz = dz;
            this.depth = depth;
        }

        public override IList<Face> ToFaces()
        {
            double magSq = dx * dx + dy * dy + dz * dz;
            double mag = Math.Sqrt(magSq);
            Tuple<double, double, double> extrusion =
                Tuple.Create(
                    dx / mag * depth,
                    dy / mag * depth,
                    dz / mag * depth);

            // The normal of the base should be "out," but there's no guarantee
            // that the normal of the defining area is "out". So a check is
            // necessary.
            Tuple<double, double, double> baseNorm = CalculateNormal(area);
            double baseMagSq =
                baseNorm.Item1 * baseNorm.Item1 +
                baseNorm.Item2 * baseNorm.Item2 +
                baseNorm.Item3 * baseNorm.Item3;
            double combinedX = baseNorm.Item1 + dx;
            double combinedY = baseNorm.Item2 + dy;
            double combinedZ = baseNorm.Item3 + dz;
            double combinedMagSq =
                combinedX * combinedX +
                combinedY * combinedY +
                combinedZ * combinedZ;
            Face usedBase;
            if (combinedMagSq < baseMagSq + magSq) { usedBase = area; }
            else { usedBase = area.Reversed(); }

            Face extruded =
                usedBase.Translated(
                    extrusion.Item1,
                    extrusion.Item2,
                    extrusion.Item3);

            List<Face> res = new List<Face>();

            Func<Polyloop, Polyloop, IEnumerable<Polyloop>> getSides =
                (source, target) =>
                {
                    List<LinkedList<Point>> sides =
                        new List<LinkedList<Point>>();
                    sides.AddRange(
                        source.Vertices.Select(_ => new LinkedList<Point>()));
                    for (int i = 0; i < source.Vertices.Count; ++i)
                    {
                        int nextI = (i + 1) % source.Vertices.Count;
                        sides[i].AddLast(target.Vertices[i]);
                        sides[i].AddLast(source.Vertices[i]);
                        sides[nextI].AddFirst(target.Vertices[i]);
                        sides[nextI].AddFirst(source.Vertices[i]);
                    }
                    return sides.Select(points => new Polyloop(points));
                };

            res.Add(usedBase);
            res.Add(extruded.Reversed());
            res.AddRange(
                getSides(usedBase.OuterBoundary, extruded.OuterBoundary)
                .Select(loop => new Face(loop)));

            res.AddRange(this.area.Voids.SelectMany(v =>
            {
                Polyloop target =
                    v.Translated(
                        extrusion.Item1,
                        extrusion.Item2,
                        extrusion.Item3);
                return getSides(v, target).Select(loop => new Face(loop));
            }));

            return res;
        }

        private static Tuple<double, double, double> CalculateNormal(Face f)
        {
            // http://cs.haifa.ac.il/~gordon/plane.pdf
            double a = 0;
            double b = 0;
            double c = 0;
            int count = f.OuterBoundary.Vertices.Count;
            for (int i = 0; i < count; ++i)
            {
                Point curr = f.OuterBoundary.Vertices[i];
                Point next = f.OuterBoundary.Vertices[(i + 1) % count];
                a += (curr.Y - next.Y) * (curr.Z + next.Z);
                b += (curr.Z - next.Z) * (curr.X + next.X);
                c += (curr.X - next.X) * (curr.Y + next.Y);
            }
            return Tuple.Create(a, b, c);
        }

        internal override Solid ScaledBy(double factor)
        {
            var newArea = area.ScaledBy(factor);
            return new ExtrudedAreaSolid(newArea, dx, dy, dz, depth * factor);
        }

        internal override NativeCoreTypes.Solid ToNative()
        {
            NativeCoreTypes.Solid native = new NativeCoreTypes.Solid();
            native.repType = NativeCoreTypes.SolidRepType.ExtrudedAreaSolid;
            native.asExt.area = Area.ToNative();
            native.asExt.dx = Dx;
            native.asExt.dy = Dy;
            native.asExt.dz = Dz;
            native.asExt.depth = Depth;
            return native;
        }
    }
}
