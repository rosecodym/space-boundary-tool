using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Data;

using Sbt.CoreTypes;

using Normal = System.Tuple<double, double, double>;

namespace GUI
{
    class SbtBuildingInformation
    {
        public string IfcFilename { get; set; }
        public IList<ElementInfo> Elements { get; set; }
        public IList<SpaceInfo> Spaces { get; set; }
        public SpaceBoundaryCollection SpaceBoundaries { get; set; }
        public IList<Normal> CompositeDirections { get; set; }
        public int PointCount { get; set; }
        public int EdgeCount { get; set; }
        public int FaceCount { get; set; }
        public int SolidCount { get; set; }
    }
}
