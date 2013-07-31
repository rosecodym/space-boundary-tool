using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Sbt.CoreTypes
{
    public enum ElementType : int
    {
        Wall,
        Slab,
        Door,
        Window,
        Column,
        Beam,
        Unknown
    }

    public class ElementInfo
    {
        private readonly string guid;
        private readonly ElementType type;
        private readonly int materialId;
        private readonly Solid geometry;

        public string Guid { get { return guid; } }
        public ElementType Type { get { return type; } }
        public int MaterialId { get { return materialId; } }
        public Solid Geometry { get { return geometry; } }

        internal ElementInfo(NativeCoreTypes.ElementInfo fromNative)
        {
            guid = fromNative.guid;
            type = (ElementType)fromNative.type;
            materialId = fromNative.materialId;
            geometry = Solid.FromNative(fromNative.geometry);
        }

        public ElementInfo(string guid, ElementType type, int mat, Solid geom)
        {
            this.guid = guid;
            this.type = type;
            this.materialId = mat;
            this.geometry = geom;
        }

        public ElementInfo WithScaledGeometry(double factor)
        {
            var newGeom = geometry.ScaledBy(factor);
            return new ElementInfo(guid, type, materialId, newGeom);
        }

        public static IEnumerable<ElementInfo> CreateElementsFromLayers(
            string guid, 
            ElementType type, 
            ICollection<MaterialLayer> layers, 
            Tuple<double, double, double> dir, 
            Solid geometry)
        {
            if (layers.Count == 1)
            {
                List<ElementInfo> single = new List<ElementInfo>();
                var firstID = layers.First().Id;
                single.Add(new ElementInfo(guid, type, firstID, geometry));
                return single;
            }
            else
            {
                throw new NotImplementedException(
                    "Multilayer elements cannot be split.");
            }
        }

        public bool IsFenestration
        {
            get
            {
                return Type == ElementType.Window || Type == ElementType.Door;
            }
        }

        internal NativeCoreTypes.ElementInfo ToNative()
        {
            NativeCoreTypes.ElementInfo native;
            native.guid = Guid;
            native.type = (NativeCoreTypes.ElementType)Type;
            native.materialId = MaterialId;
            native.geometry = Geometry.ToNative();
            return native;
        }
    }
}
