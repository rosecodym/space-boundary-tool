using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using Sbt.CoreTypes;

namespace GUI.Operations
{
    static partial class IdfGeneration
    {
        class BuildingSurface
        {
            public enum SurfaceType
            {
                Floor,
                Wall,
                Ceiling,
                Roof
            }

            public enum OtherSideConditionType
            {
                Adiabatic,
                Surface,
                Outdoors,
                Ground
            }

            readonly SpaceBoundary sbtInfo;
            readonly string constructionName;
            readonly string zoneName;
            readonly bool connectedToGround;

            public BuildingSurface(SpaceBoundary sb, string constructionName, string zoneName, bool connectedToGround)
            {
                this.sbtInfo = sb;
                this.constructionName = constructionName;
                this.zoneName = zoneName;
                this.connectedToGround = connectedToGround;
            }

            public string Name { get { return sbtInfo.Guid; } }
            public string ZoneName { get { return zoneName; } }
            public string ConstructionName { get { return constructionName; } }
            public SurfaceType Type
            {
                get
                {
                    if (Normal.Item1 != 0 || Normal.Item2 != 0) { return SurfaceType.Wall; }
                    else if (Normal.Item3 < 0) { return SurfaceType.Floor; }
                    else { return OtherSideCondition == OtherSideConditionType.Outdoors ? SurfaceType.Roof : SurfaceType.Ceiling; }
                }
            }
            public OtherSideConditionType OtherSideCondition
            {
                get
                {
                    return
                        connectedToGround ? OtherSideConditionType.Ground :
                        sbtInfo.IsExternal ? OtherSideConditionType.Outdoors :
                        sbtInfo.Opposite != null && sbtInfo.Opposite.BoundedSpace != sbtInfo.BoundedSpace ? OtherSideConditionType.Surface : OtherSideConditionType.Adiabatic;
                }
            }

            public bool IsVirtual { get { return sbtInfo.IsVirtual; } }
            public string ElementGuid { get { return sbtInfo.Element != null ? sbtInfo.Element.Guid : null; } }
            public string OtherSideName
            {
                get
                {
                    return sbtInfo.Opposite != null ? sbtInfo.Opposite.Guid : null;
                }
            }
            public Sbt.CoreTypes.Polyloop Geometry { get { return sbtInfo.Geometry.Cleaned(0.01); } }
            public Tuple<double, double, double> Normal { get { return sbtInfo.Normal; } }
        }
    }
}
