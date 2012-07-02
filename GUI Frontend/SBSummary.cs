using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace GUI_Frontend
{
    public class SBSummary
    {
        string spaceGuid;
        int level2PhysicalInternal;
        int level2PhysicalExternal;
        int level3Internal;
        int level3External;
        int level4;
        int level5;
        int virt;

        private SBSummary(string guid, int l2pi, int l2pe, int l3i, int l3e, int l4, int l5, int v)
        {
            spaceGuid = guid;
            level2PhysicalInternal = l2pi;
            level2PhysicalExternal = l2pe;
            level3Internal = l3i;
            level3External = l3e;
            level4 = l4;
            level5 = l5;
            virt = v;
        }

        internal string SpaceGuid { get { return spaceGuid; } }
        internal int Level2PhysicalInternal { get { return level2PhysicalInternal; } }
        internal int Level2PhysicalExternal { get { return level2PhysicalExternal; } }
        internal int Level3Internal { get { return level3Internal; } }
        internal int Level3External { get { return level3External; } }
        internal int Level4 { get { return level4; } }
        internal int Level5 { get { return Level5; } }
        internal int Virt { get { return virt; } }

        internal static List<SBSummary> GetAllSpaceSummaries(SBTInterface.SBCounts counts)
        {
            return new List<SBSummary>();
        }
    }
}
