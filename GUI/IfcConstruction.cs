using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;

namespace GUI
{
    class IfcConstruction : INotifyPropertyChanged
    {
        IfcInformationExtractor.Construction ifcInformation;
        Constructions.MaterialLayer idfMappingTarget;
        bool? participatesInSpaceBoundary;

        public event PropertyChangedEventHandler PropertyChanged;

        public string Name { get { return ifcInformation.Name; } }
        public bool IsComposite { get { return ifcInformation.IsComposite; } }

        public bool? ParticipatesInSpaceBoundary
        {
            get { return participatesInSpaceBoundary; }
            set
            {
                participatesInSpaceBoundary = value;
                if (PropertyChanged != null) { PropertyChanged(this, new PropertyChangedEventArgs("ParticipatesInSpaceBoundary")); }
            }
        }

        public Constructions.MaterialLayer IdfMappingTarget
        {
            get { return idfMappingTarget; }
            set
            {
                idfMappingTarget = value;
                if (PropertyChanged != null) { PropertyChanged(this, new PropertyChangedEventArgs("IdfMappingTarget")); }
            }
        }

        public IfcConstruction(IfcInformationExtractor.Construction info)
        {
            this.ifcInformation = info;
        }
    }
}
