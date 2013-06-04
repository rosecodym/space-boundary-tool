using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using SpaceBoundary = Sbt.CoreTypes.SpaceBoundary;

namespace GUI
{
    class SpaceBoundaryCollection : IEnumerable<SpaceBoundary>
    {
        private readonly ICollection<SpaceBoundary> spaceBoundaries;

        private readonly int count2ndLevelPhysicalInternal = 0;
        private readonly int count2ndLevelPhysicalExternal = 0;
        private readonly int count3rdLevel = 0;
        private readonly int count4thLevel = 0;
        private readonly int countVirtual = 0;

        public SpaceBoundaryCollection(ICollection<SpaceBoundary> spaceBoundaries)
        {
            this.spaceBoundaries = spaceBoundaries;
            foreach (SpaceBoundary sb in spaceBoundaries)
            {
                if (sb.IsVirtual) { ++countVirtual; }
                else if (sb.Level == 4) { ++count4thLevel; }
                else if (sb.Level != 2) { ++count3rdLevel; }
                else if (sb.IsExternal) { ++count2ndLevelPhysicalExternal; }
                else { ++count2ndLevelPhysicalInternal; }
            }
        }

        public int SecondLevelPhysicalInternalCount { get { return count2ndLevelPhysicalInternal; } }
        public int SecondLevelPhysicalExternalCount { get { return count2ndLevelPhysicalExternal; } }
        public int ThirdLevelCount { get { return count3rdLevel; } }
        public int FourthLevelCount { get { return count4thLevel; } }
        public int VirtualCount { get { return countVirtual; } }

        public IEnumerator<SpaceBoundary> GetEnumerator()
        {
            return spaceBoundaries.GetEnumerator();
        }

        System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
        {
            return spaceBoundaries.GetEnumerator();
        }
    }
}
