using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;

namespace Sbt
{
    static class SolidSplitter
    {
        [DllImport("SolidSplitter.dll", EntryPoint = "split_solid", CallingConvention = CallingConvention.Cdecl, SetLastError = true)]
        //private static extern void SplitSolid(NativeCoreTypes.Solid s, double dx, double dy, double dz, double[] thicknesses, int layerCount, ref NativeCoreTypes.Solid[] results);
        private static extern int SplitSolid(NativeCoreTypes.Solid s, double dx, double dy, double dz, double[] thicknesses, int layerCount, IntPtr results);

        [DllImport("SolidSplitter.dll", EntryPoint = "free_created_solids", CallingConvention = CallingConvention.Cdecl, SetLastError = true)]
        private static extern void FreeCreatedSolids(NativeCoreTypes.Solid[] solids, int count);

        internal static IEnumerable<CoreTypes.Solid> SplitSolid(CoreTypes.Solid s, ICollection<CoreTypes.MaterialLayer> layers, Tuple<double, double, double> layersDir)
        {
            NativeCoreTypes.Solid nativeSolid = s.ToNative();
            double[] thicknesses = new double[layers.Count];
            NativeCoreTypes.Solid[] results = new NativeCoreTypes.Solid[layers.Count];
            int i = 0;
            foreach (CoreTypes.MaterialLayer layer in layers)
            {
                thicknesses[i++] = layer.Thickness;
            }
            //SplitSolid(nativeSolid, layersDir.Item1, layersDir.Item2, layersDir.Item3, thicknesses, layers.Count, ref results);
            GCHandle resHandle = GCHandle.Alloc(results, GCHandleType.Pinned);
            SplitSolid(nativeSolid, layersDir.Item1, layersDir.Item2, layersDir.Item3, thicknesses, layers.Count, resHandle.AddrOfPinnedObject());
            resHandle.Free();
           // IEnumerable<CoreTypes.Solid> res = new List<CoreTypes.Solid>(results.Select(result => CoreTypes.Solid.FromNative(result)));
            List<CoreTypes.Solid> res = new List<CoreTypes.Solid>();
            foreach (NativeCoreTypes.Solid solid in results)
            {
                res.Add(CoreTypes.Solid.FromNative(solid));
            }
            FreeCreatedSolids(results, layers.Count);
            return res;
        }
    }
}
