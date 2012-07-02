using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;

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

    public class Point : IEquatable<Point>
    {
        private readonly double x_;
        private readonly double y_;
        private readonly double z_;

        public double X { get { return x_; } }
        public double Y { get { return y_; } }
        public double Z { get { return z_; } }

        public Point(double x, double y, double z)
        {
            x_ = x;
            y_ = y;
            z_ = z;
        }

        internal Point(NativeCoreTypes.Point p)
        {
            x_ = Math.Round(p.x, 3);
            y_ = Math.Round(p.y, 3);
            z_ = Math.Round(p.z, 3);
        }

        internal Point ScaledBy(double factor)
        {
            return new Point(x_ * factor, y_ * factor, z_ * factor);
        }

        internal NativeCoreTypes.Point ToNative()
        {
            NativeCoreTypes.Point native;
            native.x = X;
            native.y = Y;
            native.z = Z;
            return native;
        }

        public override bool Equals(object obj)
        {
            return this.Equals(obj as Point);
        }

        public bool Equals(Point other)
        {
            if (Object.ReferenceEquals(other, null))
            {
                return false;
            }
            if (Object.ReferenceEquals(other, this))
            {
                return true;
            }
            if (this.GetType() != other.GetType())
            {
                return false;
            }
            return X == other.X && Y == other.Y && Z == other.Z;
        }

        public override int GetHashCode()
        {
            return X.GetHashCode() ^ Y.GetHashCode() ^ Z.GetHashCode();
        }

        public static bool operator ==(Point lhs, Point rhs)
        {
            if (Object.ReferenceEquals(lhs, null))
            {
                return Object.ReferenceEquals(rhs, null);
            }
            return lhs.Equals(rhs);
        }

        public static bool operator !=(Point lhs, Point rhs)
        {
            return !(lhs == rhs);
        }
    }

    public class Polyloop
    {
        private readonly List<Point> vertices;

        public IList<Point> Vertices
        {
            get
            {
                return vertices.AsReadOnly();
            }
        }

        public Polyloop(IEnumerable<Point> points)
        {
            vertices = new List<Point>(points);
        }

        internal Polyloop(NativeCoreTypes.Polyloop fromNative)
        {
            vertices = new List<Point>();
            IntPtr firstNativePoint = fromNative.vertices;
            for (int i = 0; i < (int)fromNative.vertexCount; ++i)
            {
                NativeCoreTypes.Point currPoint = (NativeCoreTypes.Point)Marshal.PtrToStructure(firstNativePoint + Marshal.SizeOf(typeof(NativeCoreTypes.Point)) * i, typeof(NativeCoreTypes.Point));
                vertices.Add(new Point(currPoint));
            }
        }

        public Polyloop Reversed()
        {
            return new Polyloop(Vertices.Reverse());
        }

        internal Polyloop ScaledBy(double factor)
        {
            return new Polyloop(vertices.Select(p => p.ScaledBy(factor)));
        }

        internal Polyloop Translated(double dx, double dy, double dz)
        {
            return new Polyloop(vertices.Select(p => new Point(p.X + dx, p.Y + dy, p.Z + dz)));
        }

        internal NativeCoreTypes.Polyloop ToNative()
        {
            NativeCoreTypes.Polyloop native;
            native.vertexCount = (uint)Vertices.Count;
            native.vertices = Marshal.AllocHGlobal(Vertices.Count * Marshal.SizeOf(typeof(NativeCoreTypes.Point)));
            for (int i = 0; i < Vertices.Count; ++i)
            {
                Marshal.StructureToPtr(vertices[i].ToNative(), native.vertices + Marshal.SizeOf(typeof(NativeCoreTypes.Point)) * i, false);
            }
            return native;
        }
    }

    public class Face
    {
        private readonly Polyloop outerBoundary;
        private readonly List<Polyloop> voids;

        public Polyloop OuterBoundary { get { return outerBoundary; } }

        public IList<Polyloop> Voids
        {
            get
            {
                return voids.AsReadOnly();
            }
        }

        public Face(Polyloop outerBoundary, IEnumerable<Polyloop> vs = null)
        {
            this.outerBoundary = outerBoundary;
            voids = vs != null ? new List<Polyloop>(vs) : new List<Polyloop>();
        }

        internal Face(NativeCoreTypes.Face fromNative)
        {
            this.outerBoundary = new Polyloop(fromNative.outerBoundary);
            voids = new List<Polyloop>();
            for (int i = 0; i < fromNative.voidCount; ++i)
            {
                NativeCoreTypes.Polyloop loop = (NativeCoreTypes.Polyloop)Marshal.PtrToStructure(fromNative.voids + Marshal.SizeOf(typeof(NativeCoreTypes.Polyloop)) * i, typeof(NativeCoreTypes.Polyloop));
                voids.Add(new Polyloop(loop));
            }
        }

        public Face Reversed()
        {
            return new Face(OuterBoundary.Reversed(), voids.Select(loop => loop.Reversed()));
        }

        internal Face ScaledBy(double factor)
        {
            return new Face(outerBoundary.ScaledBy(factor), voids.Select(v => v.ScaledBy(factor)));
        }

        internal Face Translated(double dx, double dy, double dz)
        {
            return new Face(this.OuterBoundary.Translated(dx, dy, dz), voids.Select(v => v.Translated(dx, dy, dz)));
        }

        internal NativeCoreTypes.Face ToNative()
        {
            NativeCoreTypes.Face native;
            native.outerBoundary = OuterBoundary.ToNative();
            native.voidCount = (uint)voids.Count;
            if (native.voidCount > 0)
            {
                native.voids = Marshal.AllocHGlobal(voids.Count * Marshal.SizeOf(typeof(NativeCoreTypes.Polyloop)));
                for (int i = 0; i < voids.Count; ++i)
                {
                    Marshal.StructureToPtr(voids[i].ToNative(), native.voids + Marshal.SizeOf(typeof(NativeCoreTypes.Polyloop)) * i, false);
                }
            }
            else
            {
                native.voids = IntPtr.Zero;
            }
            return native;
        }

    }

    public abstract class Solid
    {
        public abstract IList<Face> ToFaces();

        internal abstract Solid ScaledBy(double factor);
        internal abstract NativeCoreTypes.Solid ToNative();

        internal static Solid FromNative(NativeCoreTypes.Solid s)
        {
            if (s.repType == NativeCoreTypes.SolidRepType.ExtrudedAreaSolid)
            {
                return new ExtrudedAreaSolid(s.asExt);
            }
            else
            {
                return new Brep(s.asBrep);
            }
        }
    }

    public class Brep : Solid
    {
        private readonly List<Face> faces;
        public IList<Face> Faces
        {
            get
            {
                return faces.AsReadOnly();
            }
        }

        public override IList<Face> ToFaces()
        {
            return Faces;
        }

        internal Brep(NativeCoreTypes.Brep fromNative)
        {
            faces = new List<Face>();
            for (int i = 0; i < fromNative.faceCount; ++i)
            {
                NativeCoreTypes.Face f = (NativeCoreTypes.Face)Marshal.PtrToStructure(fromNative.faces + i * Marshal.SizeOf(typeof(NativeCoreTypes.Face)), typeof(NativeCoreTypes.Face));
                faces.Add(new Face(f));
            }
        }

        public Brep(IEnumerable<Face> fcs)
        {
            faces = new List<Face>(fcs);
        }

        internal override Solid ScaledBy(double factor)
        {
            return new Brep(faces.Select(f => f.ScaledBy(factor)));
        }

        internal override NativeCoreTypes.Solid ToNative()
        {
            NativeCoreTypes.Solid native = new NativeCoreTypes.Solid(); // because of the union?
            native.repType = NativeCoreTypes.SolidRepType.Brep;
            native.asBrep.faceCount = (uint)faces.Count;
            native.asBrep.faces = Marshal.AllocHGlobal(faces.Count * Marshal.SizeOf(typeof(NativeCoreTypes.Face)));
            for (int i = 0; i < faces.Count; ++i)
            {
                Marshal.StructureToPtr(faces[i].ToNative(), native.asBrep.faces + Marshal.SizeOf(typeof(NativeCoreTypes.Face)) * i, false);
            }
            return native;
        }
    }

    public class ExtrudedAreaSolid : Solid
    {
        private readonly Face area;
        private readonly double dx;
        private readonly double dy;
        private readonly double dz;
        private readonly double depth;

        public Face Area { get { return area; } }
        public double Dx { get { return dx; } }
        public double Dy { get { return dy; } }
        public double Dz { get { return dz; } }
        public double Depth { get { return depth; } }

        internal ExtrudedAreaSolid(NativeCoreTypes.ExtrudedAreaSolid fromNative)
            : this(new Face(fromNative.area), fromNative.dx, fromNative.dy, fromNative.dz, fromNative.depth)
        { }

        public ExtrudedAreaSolid(Face area, double dx, double dy, double dz, double depth)
        {
            this.area = area;
            this.dx = dx;
            this.dy = dy;
            this.dz = dz;
            this.depth = depth;
        }

        public override IList<Face> ToFaces()
        {
            double mag = Math.Sqrt(dx * dx + dy * dy + dz * dz);
            Tuple<double, double, double> extrusion = Tuple.Create(dx / mag * depth, dy / mag * depth, dz / mag * depth);
            Face extruded = this.area.Translated(extrusion.Item1, extrusion.Item2, extrusion.Item3);

            List<Face> res = new List<Face>();

            Func<Polyloop, Polyloop, IEnumerable<Polyloop>> getSides = (source, target) =>
                {
                    List<LinkedList<Point>> sides = new List<LinkedList<Point>>();
                    sides.AddRange(source.Vertices.Select(_ => new LinkedList<Point>()));
                    for (int i = 0; i < source.Vertices.Count; ++i)
                    {
                        sides[i].AddLast(target.Vertices[i]);
                        sides[i].AddLast(source.Vertices[i]);
                        sides[(i + 1) % source.Vertices.Count].AddFirst(target.Vertices[i]);
                        sides[(i + 1) % source.Vertices.Count].AddFirst(source.Vertices[i]);
                    }
                    return sides.Select(points => new Polyloop(points));
                };

            res.Add(this.area);
            res.Add(extruded.Reversed());
            res.AddRange(getSides(this.area.OuterBoundary, extruded.OuterBoundary).Select(loop => new Face(loop)));

            res.AddRange(this.area.Voids.SelectMany(v =>
                {
                    Polyloop target = v.Translated(extrusion.Item1, extrusion.Item2, extrusion.Item3);
                    return getSides(v, target).Select(loop => new Face(loop));
                }));

            return res;
        }

        internal override Solid ScaledBy(double factor)
        {
            return new ExtrudedAreaSolid(area.ScaledBy(factor), dx, dy, dz, depth * factor);
        }

        internal override NativeCoreTypes.Solid ToNative()
        {
            NativeCoreTypes.Solid native = new NativeCoreTypes.Solid();
            native.repType = NativeCoreTypes.SolidRepType.ExtrudedAreaSolid;
            native.asExt.area = Area.ToNative();
            native.asExt.dx = Dx;
            native.asExt.dy = Dy;
            native.asExt.dz = Dz;
            native.asExt.depth = Depth;
            return native;
        }
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

        public ElementInfo(string guid, ElementType type, int material, Solid geometry)
        {
            this.guid = guid;
            this.type = type;
            this.materialId = material;
            this.geometry = geometry;
        }

        public ElementInfo WithScaledGeometry(double factor)
        {
            return new ElementInfo(guid, type, materialId, geometry.ScaledBy(factor));
        }

        public static IEnumerable<ElementInfo> CreateElementsFromLayers(string guid, ElementType type, ICollection<MaterialLayer> layers, Tuple<double, double, double> dir, Solid geometry)
        {
            if (layers.Count == 1)
            {
                List<ElementInfo> single = new List<ElementInfo>();
                single.Add(new ElementInfo(guid, type, layers.ElementAt(0).Id, geometry));
                return single;
            }
            else
            {
                return
                    SolidSplitter.SplitSolid(geometry, layers, dir)
                    .Zip(layers, (s, layer) => Tuple.Create(s, layer))
                    .Select(tuple => new ElementInfo(guid, type, tuple.Item2.Id, tuple.Item1));
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

    public class SpaceInfo
    {
        private readonly string guid;
        private readonly Solid geometry;

        public string Guid { get { return guid; } }
        public Solid Geometry { get { return geometry; } }

        internal SpaceInfo(NativeCoreTypes.SpaceInfo fromNative)
        {
            guid = fromNative.guid;
            geometry = Solid.FromNative(fromNative.geometry);
        }

        public SpaceInfo(string guid, Solid geometry)
        {
            this.guid = guid;
            this.geometry = geometry;
        }

        public SpaceInfo WithScaledGeometry(double factor)
        {
            return new SpaceInfo(guid, geometry.ScaledBy(factor));
        }

        internal NativeCoreTypes.SpaceInfo ToNative()
        {
            NativeCoreTypes.SpaceInfo native;
            native.guid = Guid;
            native.geometry = Geometry.ToNative();
            return native;
        }
    }

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

    public class SpaceBoundary
    {
        public string Guid { get; private set; }
        public ElementInfo Element { get; private set; }
        public Polyloop Geometry { get; private set; }
        public Tuple<double, double, double> Normal { get; private set; }
        public int Level { get; private set; }
        public SpaceInfo BoundedSpace { get; private set; }
        public SpaceBoundary Opposite { get; private set; }
        public SpaceBoundary ContainingBoundary { get; private set; }
        public bool LiesOnOutside { get; private set; }
        public bool IsVirtual { get; private set; }
        public bool IsExternal
        {
            get
            {
                return Opposite != null && Opposite.LiesOnOutside;
            }
        }
        private List<MaterialLayer> materialLayers;
        public IList<MaterialLayer> MaterialLayers
        {
            get
            {
                return materialLayers.AsReadOnly();
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
            Normal = Tuple.Create(fromNative.normalX, fromNative.normalY, fromNative.normalZ);
            Level = fromNative.level;
            LiesOnOutside = fromNative.liesOnOutside != 0;
            IsVirtual = fromNative.isVirtual != 0;
            int[] materialIds = new int[fromNative.materialLayerCount];
            double[] thicknesses = new double[fromNative.materialLayerCount];
            if (fromNative.materialLayerCount > 0)
            {
                Marshal.Copy(fromNative.layers, materialIds, 0, (int)fromNative.materialLayerCount);
                Marshal.Copy(fromNative.thicknesses, thicknesses, 0, (int)fromNative.materialLayerCount);
            }
            materialLayers = new List<MaterialLayer>(materialIds.Zip(thicknesses, (id, thickness) => new MaterialLayer(id, Math.Round(thickness, 3))));
            if (fromNative.spaceInfo != IntPtr.Zero)
            {
                NativeCoreTypes.SpaceInfo space = (NativeCoreTypes.SpaceInfo)Marshal.PtrToStructure(fromNative.spaceInfo, typeof(NativeCoreTypes.SpaceInfo));
                spaceGuid = space.guid;
            }
            if (fromNative.opposite != IntPtr.Zero)
            {
                NativeCoreTypes.SpaceBoundary opposite = (NativeCoreTypes.SpaceBoundary)Marshal.PtrToStructure(fromNative.opposite, typeof(NativeCoreTypes.SpaceBoundary));
                oppositeGuid = opposite.guid;
            }
            if (fromNative.parent != IntPtr.Zero)
            {
                NativeCoreTypes.SpaceBoundary parent = (NativeCoreTypes.SpaceBoundary)Marshal.PtrToStructure(fromNative.parent, typeof(NativeCoreTypes.SpaceBoundary));
                parentGuid = parent.guid;
            }
        }

        public void ScaleGeometry(double factor)
        {
            Geometry = Geometry.ScaledBy(factor);
            materialLayers = new List<MaterialLayer>(materialLayers.Select(layer => layer.WithScaledGeometry(factor)));
        }

        public override int GetHashCode()
        {
            return Guid.GetHashCode();
        }

        static public void LinkOpposites(ICollection<SpaceBoundary> allBoundaries)
        {
            if (allBoundaries.Count > 0)
            {
                Dictionary<string, SpaceBoundary> dict = new Dictionary<string, SpaceBoundary>();
                foreach (SpaceBoundary b in allBoundaries)
                {
                    dict[b.Guid] = b;
                }
                foreach (SpaceBoundary b in allBoundaries)
                {
                    if (b.oppositeGuid != null)
                    {
                        b.Opposite = dict[b.oppositeGuid];
                    }
                }
            }
        }

        static public void LinkContaining(ICollection<SpaceBoundary> allBoundaries)
        {
            if (allBoundaries.Count > 0)
            {
                Dictionary<string, SpaceBoundary> dict = new Dictionary<string, SpaceBoundary>();
                foreach (SpaceBoundary b in allBoundaries)
                {
                    dict[b.Guid] = b;
                }
                foreach (SpaceBoundary b in allBoundaries)
                {
                    if (b.parentGuid != null)
                    {
                        b.ContainingBoundary = dict[b.parentGuid];
                    }
                }
            }
        }

        static public void LinkSpaces(IEnumerable<SpaceBoundary> allBoundaries, IEnumerable<SpaceInfo> allSpaces)
        {
            if (allBoundaries.Any(_ => true) && allSpaces.Any(_ => true))
            {
                Dictionary<string, SpaceInfo> spaceDict = new Dictionary<string, SpaceInfo>();
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

        static public void LinkElements(IEnumerable<SpaceBoundary> allBoundaries, IEnumerable<ElementInfo> allElements)
        {
            if (allBoundaries.Any(_ => true) && allElements.Any(_ => true))
            {
                Dictionary<string, ElementInfo> dict = new Dictionary<string, ElementInfo>();
                foreach (ElementInfo e in allElements)
                {
                    dict[e.Guid] = e;
                }
                foreach (SpaceBoundary b in allBoundaries.Where(x => !x.IsVirtual))
                {
                    b.Element = dict[b.elementGuid];
                }
            }
        }
    }

    internal class ElementList : IDisposable
    {
        readonly int stride = Marshal.SizeOf(typeof(IntPtr));
        internal int Count { get; private set; }
        internal IntPtr NativePtr { get; private set; }
        internal ElementList(ICollection<ElementInfo> elements)
        {
            Count = elements.Count;
            if (Count > 0)
            {
                NativePtr = Marshal.AllocHGlobal(Count * stride);
                int i = 0;
                foreach (ElementInfo e in elements)
                {
                    Marshal.WriteIntPtr(NativePtr, i++ * stride, e.ToNative().Alloc());
                }
            }
            else
            {
                NativePtr = IntPtr.Zero;
            }
        }

        public void Dispose()
        {
            for (int i = 0; i < Count; ++i)
            {
                IntPtr element = Marshal.ReadIntPtr(NativePtr, i * stride);
                ((NativeCoreTypes.ElementInfo)Marshal.PtrToStructure(element, typeof(NativeCoreTypes.ElementInfo))).Release();
                Marshal.FreeHGlobal(element);
            }
            Marshal.FreeHGlobal(NativePtr);
        }
    }

    internal class SpaceList : IDisposable
    {
        readonly int stride = Marshal.SizeOf(typeof(IntPtr));
        internal int Count { get; private set; }
        internal IntPtr NativePtr { get; private set; }
        internal SpaceList(ICollection<SpaceInfo> spaces)
        {
            Count = spaces.Count;
            if (Count > 0)
            {
                NativePtr = Marshal.AllocHGlobal(Count * stride);
                int i = 0;
                foreach (SpaceInfo s in spaces)
                {
                    Marshal.WriteIntPtr(NativePtr, i++ * stride, s.ToNative().Alloc());
                }
            }
            else
            {
                NativePtr = IntPtr.Zero;
            }
        }

        public void Dispose()
        {
            for (int i = 0; i < Count; ++i)
            {
                IntPtr space = Marshal.ReadIntPtr(NativePtr, i * stride);
                ((NativeCoreTypes.SpaceInfo)Marshal.PtrToStructure(space, typeof(NativeCoreTypes.SpaceInfo))).Release();
                Marshal.FreeHGlobal(space);
            }
            Marshal.FreeHGlobal(NativePtr);
        }
    }
}
