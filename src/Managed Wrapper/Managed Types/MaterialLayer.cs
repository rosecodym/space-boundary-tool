using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Sbt.CoreTypes
{
    public class MaterialLayer
    {
        private readonly int id;
        private readonly double thickness;

        public int Id { get { return id; } }
        public double Thickness { get { return thickness; } }

        public MaterialLayer(int id, double thickness)
        {
            this.id = id;
            this.thickness = thickness;
        }

        internal MaterialLayer WithScaledGeometry(double factor)
        {
            return new MaterialLayer(id, thickness * factor);
        }
    }
}
