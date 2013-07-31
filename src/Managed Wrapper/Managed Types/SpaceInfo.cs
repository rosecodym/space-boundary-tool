using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Sbt.CoreTypes
{
    public class SpaceInfo
    {
        private readonly string guid;
        private readonly Solid geometry;

        public string Guid { get { return guid; } }
        public Solid Geometry { get { return geometry; } }

        internal SpaceInfo(NativeCoreTypes.SpaceInfo fromNative)
        {
            guid = fromNative.guid;
            geometry = Solid.FromNative(fromNative.geometry);
        }

        public SpaceInfo(string guid, Solid geometry)
        {
            this.guid = guid;
            this.geometry = geometry;
        }

        public SpaceInfo WithScaledGeometry(double factor)
        {
            return new SpaceInfo(guid, geometry.ScaledBy(factor));
        }

        internal NativeCoreTypes.SpaceInfo ToNative()
        {
            NativeCoreTypes.SpaceInfo native;
            native.guid = Guid;
            native.geometry = Geometry.ToNative();
            return native;
        }
    }
}
