using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;

using Space = Sbt.NativeCoreTypes.SpaceInfo;

namespace Sbt.CoreTypes
{
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
                    var alloced = s.ToNative().Alloc();
                    Marshal.WriteIntPtr(NativePtr, i++ * stride, alloced);
                }
            }
            else { NativePtr = IntPtr.Zero; }
        }

        public void Dispose()
        {
            for (int i = 0; i < Count; ++i)
            {
                IntPtr space = Marshal.ReadIntPtr(NativePtr, i * stride);
                var s = Marshal.PtrToStructure(space, typeof(Space));
                ((Space)s).Release();
                Marshal.FreeHGlobal(space);
            }
            Marshal.FreeHGlobal(NativePtr);
        }
    }
}
