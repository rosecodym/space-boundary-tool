using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Sbt.CoreTypes
{
    public class Point : IEquatable<Point>
    {
        private readonly double x_;
        private readonly double y_;
        private readonly double z_;

        public double X { get { return x_; } }
        public double Y { get { return y_; } }
        public double Z { get { return z_; } }

        public Point(double x, double y, double z)
        {
            x_ = x;
            y_ = y;
            z_ = z;
        }

        internal Point(NativeCoreTypes.Point p)
        {
            x_ = Math.Round(p.x, 3);
            y_ = Math.Round(p.y, 3);
            z_ = Math.Round(p.z, 3);
        }

        internal Point ScaledBy(double factor)
        {
            return new Point(x_ * factor, y_ * factor, z_ * factor);
        }

        internal NativeCoreTypes.Point ToNative()
        {
            NativeCoreTypes.Point native;
            native.x = X;
            native.y = Y;
            native.z = Z;
            return native;
        }

        public override bool Equals(object obj)
        {
            return this.Equals(obj as Point);
        }

        public bool Equals(Point other)
        {
            if (Object.ReferenceEquals(other, null)) { return false; }
            if (Object.ReferenceEquals(other, this)) { return true; }
            if (this.GetType() != other.GetType()) { return false; }
            return X == other.X && Y == other.Y && Z == other.Z;
        }

        public override int GetHashCode()
        {
            return X.GetHashCode() ^ Y.GetHashCode() ^ Z.GetHashCode();
        }

        public static bool operator ==(Point lhs, Point rhs)
        {
            if (Object.ReferenceEquals(lhs, null))
            {
                return Object.ReferenceEquals(rhs, null);
            }
            return lhs.Equals(rhs);
        }

        public static bool operator !=(Point lhs, Point rhs)
        {
            return !(lhs == rhs);
        }
    }
}
