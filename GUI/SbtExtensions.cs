using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using Sbt.CoreTypes;

namespace GUI
{
    static class SbtExtensions
    {
        private class Vector3
        {
            readonly private double x;
            readonly private double y;
            readonly private double z;

            public Vector3(Point from, Point to)
            {
                x = to.X - from.X;
                y = to.Y - from.Y;
                z = to.Z - from.Z;
            }

            private Vector3(double x, double y, double z)
            {
                this.x = x;
                this.y = y;
                this.z = z;
            }

            public double Magnitude { get { return Math.Sqrt(x * x + y * y + z * z); } }

            static public Vector3 CrossProduct(Vector3 u, Vector3 v)
            {
                return new Vector3(
                    u.y * v.z - u.z * v.y,
                    u.z * v.x - u.x * v.z,
                    u.x * v.y - u.y * v.x);
            }
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
    }
}
