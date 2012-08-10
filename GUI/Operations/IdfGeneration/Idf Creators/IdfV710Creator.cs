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
        }
    }
}
