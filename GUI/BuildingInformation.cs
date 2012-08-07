using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using Sbt.CoreTypes;

namespace GUI
{
    class BuildingInformation
    {
        public List<ElementInfo> Elements { get; set; }
        public List<SpaceInfo> Spaces { get; set; }
        public SpaceBoundaryCollection SpaceBoundaries { get; set; }
    }
}
