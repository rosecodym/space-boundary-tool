using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace GUI
{
    class IfcConstruction
    {
        IfcInformationExtractor.Construction ifcInformation;

        public string Name { get { return ifcInformation.Name; } }
        public bool IsComposite { get { return ifcInformation.IsComposite; } }
        public string IdfMappingTarget { get; set; }

        public IfcConstruction(IfcInformationExtractor.Construction info)
        {
            this.ifcInformation = info;
        }
    }
}
