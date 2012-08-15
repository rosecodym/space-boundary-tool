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
        abstract class IdfCreator
        {
            protected readonly Idf idf;
            protected readonly Action<string> notify;

            protected IdfCreator(Idf idf, Action<string> notify)
            {
                this.idf = idf;
                this.notify = notify;
            }

            public void WriteToFile(string filename)
            {
                idf.Write(filename, LibIdf.Base.IdfValidationChecks.None);
            }

            public static IdfCreator Build(EnergyPlusVersion version, Idd idd, Action<string> notify)
            {
                if (version == EnergyPlusVersion.V710)
                {
                    return new IdfV710Creator(new Idf(idd), notify);
                }
                else
                {
                    throw new ArgumentException("Unsupported EnergyPlus version.\n");
                }
            }

            public abstract void AddConstantContents();
            public abstract void AddConstruction(Materials.Output.Construction c);
            public abstract void AddMaterial(Materials.Output.MaterialLayer layer);
            public abstract void AddZone(string name);
        }
    }
}
