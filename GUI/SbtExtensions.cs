using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using Sbt.CoreTypes;

namespace GUI
{
    static class SbtExtensions
    {
        public class Vector3
        {
            readonly private double x;
            readonly private double y;
            readonly private double z;

            public double X { get { return x; } }
            public double Y { get { return y; } }
            public double Z { get { return z; } }

            public Vector3(Point from, Point to)
            {
                x = to.X - from.X;
                y = to.Y - from.Y;
                z = to.Z - from.Z;
            }

            public Vector3(double x, double y, double z)
            {
                this.x = x;
                this.y = y;
                this.z = z;
            }

            public double Magnitude // pop pop
            {
                get { return Math.Sqrt(MagnitudeSquared); }
            }

            public double MagnitudeSquared // pop pop pop pop
            {
                get { return x * x + y * y + z * z; }
            }

            public static Vector3 operator +(Vector3 a, Vector3 b)
            {
                return new Vector3(
                    a.X + b.X,
                    a.Y + b.Y,
                    a.Z + b.Z);
            }

            public static bool AreAntiparallel(Vector3 a, Vector3 b)
            {
                double combined = (a + b).MagnitudeSquared;
                double separate = a.MagnitudeSquared + b.MagnitudeSquared;
                if (combined < separate)
                {
                    Vector3 cross = Vector3.CrossProduct(a, b);
                    return cross.MagnitudeSquared < 0.01 * 0.01;
                }
                return false;
            }

            public static Vector3 CrossProduct(Vector3 u, Vector3 v)
            {
                return new Vector3(
                    u.y * v.z - u.z * v.y,
                    u.z * v.x - u.x * v.z,
                    u.x * v.y - u.y * v.x);
            }
        }

        public class Plane3
        {
            readonly private double a;
            readonly private double b;
            readonly private double c;
            readonly private double d;

            public Vector3 Normal { get { return new Vector3(a, b, c); } }

            public Plane3(Polyloop loop)
            {
            	// http://cs.haifa.ac.il/~gordon/plane.pdf
                a = 0;
                b = 0;
                c = 0;
                double x = 0;
                double y = 0;
                double z = 0;
                int count = loop.Vertices.Count;
                for (int i = 0; i < count; ++i)
                {
                    Point curr = loop.Vertices[i];
                    Point next = loop.Vertices[(i + 1) % count];
                    a += (curr.Y - next.Y) * (curr.Z + next.Z);
                    b += (curr.Z - next.Z) * (curr.X + next.X);
                    c += (curr.X - next.X) * (curr.Y + next.Y);
                    x += curr.X;
                    y += curr.Y;
                    z += curr.Z;
                }
                x /= count;
                y /= count;
                z /= count;
                d = -x * a + -y * b + -z * c;
            }

            public double SignedDistanceFrom(Point p)
            {
                // http://mathworld.wolfram.com/Point-PlaneDistance.html
                return
                    (a * p.X + b * p.Y + c * p.Z + d) /
                    Math.Sqrt(a * a + b * b + c * c);
            }
        }

        public static Plane3 Plane(this Polyloop loop)
        {
            return new Plane3(loop);
        }

        public static Polyloop Cleaned(this Polyloop loop, double eps)
        {
            if (loop.Vertices.Count < 3) { return new Polyloop(new Point[] { }); }

            LinkedList<Point> pointList = new LinkedList<Point>(loop.Vertices);
            LinkedListNode<Point>[] window = new LinkedListNode<Point>[3] { pointList.First, null, null };

            Func<LinkedListNode<Point>[], bool> collapse = w =>
            {
                Vector3 bToC = new Vector3(w[1].Value, w[2].Value);
                if (bToC.Magnitude < eps)
                {
                    pointList.Remove(w[2]);
                    return true;
                }
                Vector3 aToC = new Vector3(w[0].Value, w[2].Value);
                if (aToC.Magnitude < eps)
                {
                    pointList.Remove(w[1]);
                    pointList.Remove(w[2]);
                    return true;
                }
                Vector3 aToB = new Vector3(w[0].Value, w[1].Value);
                if (aToB.Magnitude < eps || Vector3.CrossProduct(aToB, bToC).Magnitude < eps)
                {
                    pointList.Remove(w[1]);
                    return true;
                }
                return false;
            };

            Func<LinkedListNode<Point>, LinkedListNode<Point>> nextOf = n => n == pointList.Last ? pointList.First : n.Next;

            int stepsSinceLastChange = 0;

            while (pointList.Count >= 3 && stepsSinceLastChange < pointList.Count)
            {
                window[1] = nextOf(window[0]);
                window[2] = nextOf(window[1]);
                if (!collapse(window))
                {
                    window[0] = nextOf(window[0]);
                    ++stepsSinceLastChange;
                }
                else
                {
                    stepsSinceLastChange = 0;
                }
            }

            if (pointList.Count < 3) { return new Polyloop(new Point[] { }); }
            else { return new Polyloop(pointList); }
        }

        public static bool IsConnectedToGround(this SpaceBoundary sb)
        {
            return !sb.Geometry.Vertices.Any(p => p.Z > 0.0);
        }

        internal static Point Center(this Polyloop loop)
        {
            double x = 0, y = 0, z = 0;
            foreach (Point p in loop.Vertices)
            {
                x += p.X;
                y += p.Y;
                z += p.Z;
            }
            return new Point(x / loop.Vertices.Count, y / loop.Vertices.Count, z / loop.Vertices.Count);
        }

        internal static Point AverageOfFaceCenters(this Solid s)
        {
            double x = 0, y = 0, z = 0;
            IList<Face> faces = s.ToFaces();
            foreach (Sbt.CoreTypes.Face f in faces)
            {
                Sbt.CoreTypes.Point c = f.OuterBoundary.Center();
                x += c.X;
                y += c.Y;
                z += c.Z;
            }
            return new Point(x / faces.Count, y / faces.Count, z / faces.Count);
        }

        internal static Polyloop Translate(this Polyloop loop, Vector3 v)
        {
            return new Polyloop(loop.Vertices.Select(p =>
                new Point(p.X + v.X, p.Y + v.Y, p.Z + v.Z)));
        }

        public static Plane3 Plane(this SpaceBoundary sb)
        {
            return new Plane3(sb.Geometry);
        }
    }
}
