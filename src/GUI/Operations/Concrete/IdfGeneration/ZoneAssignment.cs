using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using SpaceBoundary = Sbt.CoreTypes.SpaceBoundary;
using IfcSpace = IfcInterface.IfcSpace;

namespace GUI.Operations
{
    partial class IdfGeneration
    {
        class ZoneAssignment
        {
            Dictionary<string, string> nameLookup;

            public ZoneAssignment(
                IEnumerable<SpaceBoundary> spaceBoundaries, 
                ICollection<IfcSpace> spaces)
            {
                nameLookup = new Dictionary<string, string>();
                var sbs = new List<SpaceBoundary>(spaceBoundaries);

                var usedSpaceGuids = new HashSet<string>(
                    sbs.Select(sb => sb.BoundedSpace.Guid));
                var usedSpaces = 
                    spaces.Where(sp => usedSpaceGuids.Contains(sp.Guid));
                var cmp = StringComparer.InvariantCultureIgnoreCase;
                var usedNames = new HashSet<string>(cmp);

                var unzoned = new List<IfcSpace>();
                foreach (IfcSpace sp in usedSpaces) {
                    if (sp.Zones != null)
                    {
                        nameLookup[sp.Guid] = sp.Zones.First().Name;
                        usedNames.Add(sp.Zones.First().Name);
                    }
                    else { unzoned.Add(sp); }
                }

                var getName = new Func<IfcSpace, int, string>[]
                {
                    (sp, _) => String.Format("{0} {1}", sp.LongName, sp.Name),
                    (sp, _) => sp.LongName,
                    (sp, _) => sp.Name,
                    (sp, _) => sp.Guid,
                    (sp, x) => String.Format("{0}-{1}", sp.Guid, x)
                };
                Func<string, bool> unacceptable = name =>
                    String.IsNullOrWhiteSpace(name) || 
                    usedNames.Contains(name);
                foreach (IfcSpace sp in unzoned)
                {
                    int x = 0;
                    string attempt;
                    do { attempt = getName[x > 4 ? 4 : x](sp, x - 2); }
                    while (unacceptable(attempt));
                    nameLookup[sp.Guid] = attempt;
                    usedNames.Add(attempt);
                }
            }

            public string this[string spaceGuid]
            {
                get { return nameLookup[spaceGuid]; }
            }

            public IEnumerable<string> AllZoneNames()
            {
                return nameLookup.Values.Distinct();
            }
        }
    }
}
