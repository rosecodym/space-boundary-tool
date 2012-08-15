using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using LibIdf.Idd;
using LibIdf.Idf;

namespace GUI.Operations
{
    static partial class IdfGeneration
    {
        class IdfV710Creator : IdfCreator
        {
            public IdfV710Creator(Idf idf, Action<string> notify) : base(idf, notify) { }

            public override void AddConstantContents()
            {
                IdfObject obj = idf.CreateObject("Version");
                obj.Fields["Version Identifier"].Value = "7.1";
            }

            public override void AddConstruction(Materials.Output.Construction c)
            {
                LibIdf.Idf.IdfObject obj = idf.CreateObject("Construction");
                obj.Fields["Name"].Value = c.Name;
                obj.Fields["Outside Layer"].Value = c.LayerNames[0];
                for (int i = 1; i < c.LayerNames.Count; ++i)
                {
                    obj.Fields[String.Format("Layer {0}", i + 1)].Value = c.LayerNames[i];
                }
            }

            public override void AddMaterial(Materials.Output.MaterialLayer layer)
            {
                layer.AddToIdfV710(idf);
            }

            public override void AddZone(string name)
            {
                LibIdf.Idf.IdfObject obj = idf.CreateObject("Zone");
                obj.Fields["Name"].Value = name;
            }
        }
    }
}
