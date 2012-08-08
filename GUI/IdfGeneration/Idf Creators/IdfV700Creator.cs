using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using LibIdf.Idd;
using LibIdf.Idf;

namespace GUI.IdfGeneration
{
    class IdfV700Creator : IdfCreator
    {
        public IdfV700Creator(Idd idd, Action<string> notify) : base(new Idf(idd), notify) { }
    }
}
