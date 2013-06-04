using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using Sbt.CoreTypes;
using Vector3 = GUI.SbtExtensions.Vector3;
using Plane3 = GUI.SbtExtensions.Plane3;
using Construction = ConstructionManagement.OutputConstructionBase;

namespace GUI.Operations
{
    partial class IdfGeneration
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
            readonly Polyloop geometry;
            readonly Plane3 plane;
            readonly Construction construction;
            readonly string zoneName;

            public BuildingSurface(
                SpaceBoundary sb,
                Construction construction, 
                string zoneName)
            {
                this.sbtInfo = sb;
                this.geometry = sbtInfo.Geometry.Cleaned(0.01);
                this.plane = sb.Plane();
                this.construction = construction;
                this.zoneName = zoneName;
            }

            public string Name { get { return sbtInfo.Guid; } }
            public string ZoneName { get { return zoneName; } }
            public Construction Construction { get { return construction; } }
            public BuildingSurface Opposite { get; set; }
            public bool IsAdiabaticVirtual
            {
                get
                {
                    return
                        construction.IsVirtual &&
                        OtherSideCondition == OtherSideConditionType.Adiabatic;
                }
            }
            public SurfaceType Type
            {
                get
                {
                    if (Plane.Normal.X != 0 || Plane.Normal.Y != 0)
                    {
                        return SurfaceType.Wall;
                    }
                    else if (Plane.Normal.Z < 0) { return SurfaceType.Floor; }
                    else
                    {
                    Func<bool> isOutdoors = 
                        () => 
                            OtherSideCondition == 
                            OtherSideConditionType.Outdoors;
                        return isOutdoors() ? 
                            SurfaceType.Roof : SurfaceType.Ceiling;
                    }
                }
            }
            public OtherSideConditionType OtherSideCondition
            {
                get
                {
                    if (sbtInfo.IsConnectedToGround())
                    {
                        return OtherSideConditionType.Ground;
                    }
                    else if (sbtInfo.IsExternal)
                    {
                        return OtherSideConditionType.Outdoors;
                    }
                    else if (
                        Opposite != null &&
                        Opposite.ZoneName != this.ZoneName)
                    {
                        return OtherSideConditionType.Surface;
                    }
                    else
                    {
                        return OtherSideConditionType.Adiabatic;
                    }
                }
            }
            public string OtherSideObject
            {
                get
                {
                    return Opposite == null ? String.Empty : Opposite.Name;
                }
            }

            public bool IsVirtual { get { return sbtInfo.IsVirtual; } }
            public string ElementGuid
            {
                get
                {
                    if (sbtInfo.Element != null)
                    {
                        return sbtInfo.Element.Guid;
                    }
                    else
                    {
                        return null;
                    }
                }
            }
            public Polyloop Geometry { get { return geometry; } }
            public Plane3 Plane { get { return plane; } }
            public Vector3 Normal { get { return plane.Normal; } }

            public bool HasElementType(ElementType type)
            {
                return
                    sbtInfo.Element != null &&
                    sbtInfo.Element.Type == type;
            }
        }
    }
}
