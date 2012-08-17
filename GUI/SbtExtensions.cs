using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using Sbt.CoreTypes;

namespace GUI
{
    static class SbtExtensions
    {
        public static Polyloop Cleaned(this Polyloop loop, double eps)
        {
            // only deals with coincidence right now
            Func<Point, Point, bool> areEffectivelySame = (a, b) =>
            {
                return (a.X - b.X) * (a.X - b.X) + (a.Y - b.Y) * (a.Y - b.Y) + (a.Z - b.Z) * (a.Z - b.Z) < (eps * eps);
            };

            LinkedList<Point> res = new LinkedList<Point>();
            foreach (Point p in loop.Vertices)
            {
                if (res.Count == 0 || !areEffectivelySame(p, res.Last.Value)) { res.AddLast(p); }
            }
            while (res.Count > 0 && areEffectivelySame(res.First.Value, res.Last.Value)) { res.RemoveLast(); }

            if (res.Count < 3) { res.Clear(); }

            return new Polyloop(res);
        }
    }
}
