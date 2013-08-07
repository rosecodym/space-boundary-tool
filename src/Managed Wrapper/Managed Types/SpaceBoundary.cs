using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;

namespace Sbt.CoreTypes
{
    public class SpaceBoundary
    {
        public string Guid { get; private set; }
        public ElementInfo Element { get; private set; }
        public Polyloop Geometry { get; private set; }
        public Tuple<double, double, double> Normal { get; private set; }
        public SpaceInfo BoundedSpace { get; private set; }
        public SpaceBoundary Opposite { get; private set; }
        public SpaceBoundary ContainingBoundary { get; private set; }
        public bool IsVirtual { get; private set; }
        public bool IsExternal { get; private set; }
        private List<MaterialLayer> materialLayers;
        public IList<MaterialLayer> MaterialLayers
        {
            get { return materialLayers.AsReadOnly(); }
        }
        public int Level
        {
            get
            {
                return
                    IsExternal ? 2 :
                    Opposite == null ? 3 :
                    BoundedSpace == Opposite.BoundedSpace ? 4 : 2;
            }
        }

        private string spaceGuid;
        private string oppositeGuid;
        private string parentGuid;
        private string elementGuid;

        internal SpaceBoundary(NativeCoreTypes.SpaceBoundary fromNative)
        {
            Guid = fromNative.guid;
            elementGuid = fromNative.elementId;
            Geometry = new Polyloop(fromNative.geometry);
            Normal = Tuple.Create(
                fromNative.normalX, 
                fromNative.normalY, 
                fromNative.normalZ);
            IsExternal = fromNative.isExternal != 0;
            IsVirtual = fromNative.isVirtual != 0;
            int[] materialIds = new int[fromNative.materialLayerCount];
            double[] thicknesses = new double[fromNative.materialLayerCount];
            if (fromNative.materialLayerCount > 0)
            {
                var layerCt = (int)fromNative.materialLayerCount;
                Marshal.Copy(fromNative.layers, materialIds, 0, layerCt);
                Marshal.Copy(fromNative.thicknesses, thicknesses, 0, layerCt);
            }
            var layers = materialIds.Zip(thicknesses, (id, thickness) =>
                new MaterialLayer(id, Math.Round(thickness, 3)));
            materialLayers = new List<MaterialLayer>(layers);
            if (fromNative.spaceInfo != IntPtr.Zero)
            {
                var natSp = fromNative.spaceInfo;
                var natSpType = typeof(NativeCoreTypes.SpaceInfo);
                var space = Marshal.PtrToStructure(natSp, natSpType);
                spaceGuid = ((NativeCoreTypes.SpaceInfo)space).guid;
            }
            if (fromNative.opposite != IntPtr.Zero)
            {
                var natOpp = fromNative.opposite;
                var natSBType = typeof(NativeCoreTypes.SpaceBoundary);
                var opp = Marshal.PtrToStructure(natOpp, natSBType);
                oppositeGuid = ((NativeCoreTypes.SpaceBoundary)opp).guid;
            }
            if (fromNative.parent != IntPtr.Zero)
            {
                var natParent = fromNative.parent;
                var natSBType = typeof(NativeCoreTypes.SpaceBoundary);
                var parent = Marshal.PtrToStructure(natParent, natSBType);
                parentGuid = ((NativeCoreTypes.SpaceBoundary)parent).guid;
            }
        }

        public void ScaleGeometry(double factor)
        {
            Geometry = Geometry.ScaledBy(factor);
            materialLayers = new List<MaterialLayer>(
                materialLayers.Select(layer => 
                    layer.WithScaledGeometry(factor)));
        }

        public override int GetHashCode() { return Guid.GetHashCode(); }

        static public void LinkOpposites(ICollection<SpaceBoundary> all)
        {
            if (all.Count > 0)
            {
                var dict = new Dictionary<string, SpaceBoundary>();
                foreach (SpaceBoundary b in all) { dict[b.Guid] = b; }
                foreach (SpaceBoundary b in all)
                {
                    if (b.oppositeGuid != null)
                    {
                        b.Opposite = dict[b.oppositeGuid];
                    }
                }
            }
        }

        static public void LinkContaining(ICollection<SpaceBoundary> all)
        {
            if (all.Count > 0)
            {
                var dict = new Dictionary<string, SpaceBoundary>();
                foreach (SpaceBoundary b in all) { dict[b.Guid] = b; }
                foreach (SpaceBoundary b in all)
                {
                    if (b.parentGuid != null)
                    {
                        b.ContainingBoundary = dict[b.parentGuid];
                    }
                }
            }
        }

        static public void LinkSpaces(
            IEnumerable<SpaceBoundary> allBoundaries, 
            IEnumerable<SpaceInfo> allSpaces)
        {
            if (allBoundaries.Any(_ => true) && allSpaces.Any(_ => true))
            {
                var spaceDict = new Dictionary<string, SpaceInfo>();
                foreach (SpaceInfo space in allSpaces)
                {
                    spaceDict[space.Guid] = space;
                }
                foreach (SpaceBoundary b in allBoundaries)
                {
                    if (b.spaceGuid != null)
                    {
                        b.BoundedSpace = spaceDict[b.spaceGuid];
                    }
                }
            }
        }

        static public void LinkElements(
            IEnumerable<SpaceBoundary> allBounds, 
            IEnumerable<ElementInfo> allElements)
        {
            if (allBounds.Any(_ => true) && allElements.Any(_ => true))
            {
                var dict = new Dictionary<string, ElementInfo>();
                foreach (ElementInfo e in allElements) { dict[e.Guid] = e; }
                foreach (SpaceBoundary b in allBounds.Where(x => !x.IsVirtual))
                {
                    b.Element = dict[b.elementGuid];
                }
            }
        }
    }
}
