using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;

using Element = Sbt.NativeCoreTypes.ElementInfo;

namespace Sbt.CoreTypes
{
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
                    var alloced = e.ToNative().Alloc();
                    Marshal.WriteIntPtr(NativePtr, i++ * stride, alloced);
                }
            }
            else { NativePtr = IntPtr.Zero; }
        }

        public void Dispose()
        {
            for (int i = 0; i < Count; ++i)
            {
                IntPtr element = Marshal.ReadIntPtr(NativePtr, i * stride);
                var e = Marshal.PtrToStructure(element, typeof(Element));
                ((Element)e).Release();
                Marshal.FreeHGlobal(element);
            }
            Marshal.FreeHGlobal(NativePtr);
        }
    }
}
