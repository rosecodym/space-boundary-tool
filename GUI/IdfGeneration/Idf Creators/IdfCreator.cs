using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using LibIdf.Idf;

namespace GUI.IdfGeneration
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
    }
}
