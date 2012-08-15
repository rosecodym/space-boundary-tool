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
                IdfObject obj;
                
                idf.CreateObject("Version").Fields["Version Identifier"].Value = "7.1";

                obj = idf.CreateObject("GlobalGeometryRules");
                obj.Fields["Starting Vertex Position"].Value = "UpperLeftCorner";
                obj.Fields["Vertex Entry Direction"].Value = "Counterclockwise";
                obj.Fields["Coordinate System"].Value = "Relative";
                
                idf.CreateObject("Output:Surfaces:Drawing").Fields["Report Type"].Value = "DXF";
                idf.CreateObject("Output:Surfaces:List").Fields["Report Type"].Value = "DetailsWithVertices";
                idf.CreateObject("Output:Surfaces:List").Fields["Report Type"].Value = "ViewFactorInfo";

                obj = idf.CreateObject("Output:Surfaces:List");
                obj.Fields["Report Type"].Value = "Lines";
                obj.Fields["Report Specifications"].Value = "IDF";

                obj = idf.CreateObject("Output:Diagnostics");
                obj.Fields["Key 1"].Value = "DoNotMirrorDetachedShading";
                obj.Fields["Key 2"].Value = "DisplayAdvancedReportVariables";

                idf.CreateObject("Output:Diagnostics").Fields["Key 1"].Value = "DisplayExtraWarnings";
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

            public override void AddZone(string name, string sourceGuid)
            {
                LibIdf.Idf.IdfObject obj = idf.CreateObject("Zone");
                obj.Fields["Name"].Value = name;
                obj.AddComment("! space GUID is " + sourceGuid);
            }
        }
    }
}
