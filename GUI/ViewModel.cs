using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Windows.Input;

using IfcConstruction = IfcInformationExtractor.Construction;
using IfcBuildingInformation = IfcInformationExtractor.BuildingInformation;
using IfcElement = IfcInformationExtractor.Element;

namespace GUI
{
    class ViewModel : INotifyPropertyChanged
    {
        private SbtBuildingInformation sbtBuilding;
        private IfcBuildingInformation ifcBuilding;
        private ICollection<Constructions.MaterialLayer> libraryMaterials = new List<Constructions.MaterialLayer>();
        private readonly IddManager idds = new IddManager();
        private bool busy = false;

        public event PropertyChangedEventHandler PropertyChanged;

        public ICommand BrowseToInputIfcFileCommand { get; private set; }
        public ICommand BrowseToOutputIfcFileCommand { get; private set; }
        public ICommand BrowseToOutputIdfFileCommand { get; private set; }
        public ICommand BrowseToMaterialsLibraryCommand { get; private set; }
        public ICommand ExecuteSbtCommand { get; private set; }
        public ICommand LoadMaterialsLibraryCommand { get; private set; }
        public ICommand LoadIfcBuildingCommand { get; private set; }
        public ICommand GenerateIdfCommand { get; private set; }

        public SbtBuildingInformation CurrentSbtBuilding
        {
            get { return sbtBuilding; }
            set
            {
                sbtBuilding = value;
                if (PropertyChanged != null)
                {
                    PropertyChanged(this, new PropertyChangedEventArgs("CurrentSbtBuilding"));
                    PropertyChanged(this, new PropertyChangedEventArgs("IdfGeneratable"));
                }
            }
        }

        public IfcBuildingInformation CurrentIfcBuilding
        {
            get { return ifcBuilding; }
            set
            {
                ifcBuilding = value;
                if (PropertyChanged != null)
                {
                    PropertyChanged(this, new PropertyChangedEventArgs("CurrentIfcBuilding"));
                    PropertyChanged(this, new PropertyChangedEventArgs("IfcConstructions"));
                }
            }
        }

        public ICollection<Constructions.MaterialLayer> LibraryMaterials
        {
            get { return libraryMaterials; }
            set
            {
                libraryMaterials = value;
                if (PropertyChanged != null) { PropertyChanged(this, new PropertyChangedEventArgs("LibraryMaterials")); }
            }
        }

        public IEnumerable<IfcConstruction> IfcConstructions
        {
            get
            {
                return ifcBuilding == null ? null : ifcBuilding.Constructions;
            }
        }

        public string InputIfcFilePath
        {
            get { return Properties.Settings.Default.InputIfcFilename; }
            set
            {
                Properties.Settings.Default.InputIfcFilename = value;
                if (PropertyChanged != null) { PropertyChanged(this, new PropertyChangedEventArgs("InputIfcFilePath")); }
            }
        }

        public string OutputIfcFilePath
        {
            get { return Properties.Settings.Default.OutputIfcFilename; }
            set
            {
                Properties.Settings.Default.OutputIfcFilename = value;
                if (PropertyChanged != null) { PropertyChanged(this, new PropertyChangedEventArgs("OutputIfcFilePath")); }
            }
        }

        public string OutputIdfFilePath
        {
            get { return Properties.Settings.Default.OutputIdfFilename; }
            set
            {
                Properties.Settings.Default.OutputIdfFilename = value;
                if (PropertyChanged != null) { PropertyChanged(this, new PropertyChangedEventArgs("OutputIdfFilePath")); }
            }
        }

        public string MaterialsLibraryPath
        {
            get { return Properties.Settings.Default.MaterialsLibraryFilename; }
            set
            {
                Properties.Settings.Default.MaterialsLibraryFilename = value;
                if (PropertyChanged != null) { PropertyChanged(this, new PropertyChangedEventArgs("MaterialsLibraryPath")); }
            }
        }

        public bool SkipWallColumnCheck
        {
            get { return Properties.Settings.Default.SkipWallColumnCheck; }
            set
            {
                Properties.Settings.Default.SkipWallColumnCheck = value;
                if (PropertyChanged != null) { PropertyChanged(this, new PropertyChangedEventArgs("SkipWallColumnCheck")); }
            }
        }

        public bool SkipSlabColumnCheck
        {
            get { return Properties.Settings.Default.SkipSlabColumnCheck; }
            set
            {
                Properties.Settings.Default.SkipSlabColumnCheck = value;
                if (PropertyChanged != null) { PropertyChanged(this, new PropertyChangedEventArgs("SkipSlabColumnCheck")); }
            }
        }

        public bool SkipWallSlabCheck
        {
            get { return Properties.Settings.Default.SkipWallSlabCheck; }
            set
            {
                Properties.Settings.Default.SkipWallSlabCheck = value;
                if (PropertyChanged != null) { PropertyChanged(this, new PropertyChangedEventArgs("SkipWallSlabCheck")); }
            }
        }

        public bool WriteIfc
        {
            get { return Properties.Settings.Default.CreateOutputIfc; }
            set
            {
                Properties.Settings.Default.CreateOutputIfc = value;
                if (PropertyChanged != null) { PropertyChanged(this, new PropertyChangedEventArgs("WriteIfc")); }
            }
        }

        public List<EnergyPlusVersion> AvailableEPVersions
        {
            get
            {
                return new List<EnergyPlusVersion>(new EnergyPlusVersion[] { EnergyPlusVersion.V710 });
            }
        }

        public int EnergyPlusVersionIndexToWrite
        {
            get { return Properties.Settings.Default.EnergyPlusVersionIndexToWrite; }
            set
            {
                Properties.Settings.Default.EnergyPlusVersionIndexToWrite = value;
                if (PropertyChanged != null) { PropertyChanged(this, new PropertyChangedEventArgs("EnergyPlusVersionIndexToWrite")); }
            }
        }

        public bool Busy
        {
            get { return busy; }
            set
            {
                if (PropertyChanged != null)
                {
                    busy = value;
                    PropertyChanged(this, new PropertyChangedEventArgs("Busy"));
                    // TODO: figure out how to make this not backwards
                    PropertyChanged(this, new PropertyChangedEventArgs("SbtInvokable"));
                    PropertyChanged(this, new PropertyChangedEventArgs("IdfGeneratable"));
                    PropertyChanged(this, new PropertyChangedEventArgs("MaterialsLibraryLoadable"));
                    PropertyChanged(this, new PropertyChangedEventArgs("IfcBuildingLoadable"));
                }
            }
        }

        public bool SbtInvokable
        {
            get { return !Busy; }
        }

        public bool IdfGeneratable
        {
            get { return !Busy && sbtBuilding != null && OutputIdfFilePath != String.Empty; }
        }

        public bool MaterialsLibraryLoadable
        {
            get { return !Busy; }
        }

        public bool IfcBuildingLoadable
        {
            get { return !Busy && InputIfcFilePath != String.Empty; }
        }

        public IddManager Idds { get { return idds; } }

        public ViewModel(Action<string> updateOutputDirectly)
        {
            BrowseToInputIfcFileCommand = new RelayCommand(_ => Operations.Miscellaneous.BrowseToInputIfcFile(this));
            BrowseToOutputIfcFileCommand = new RelayCommand(_ => Operations.Miscellaneous.BrowseToOutputIfcFile(this), _ => this.WriteIfc);
            BrowseToOutputIdfFileCommand = new RelayCommand(_ => Operations.Miscellaneous.BrowseToOutputIdfFile(this));
            BrowseToMaterialsLibraryCommand = new RelayCommand(_ => Operations.Miscellaneous.BrowseToMaterialsLibrary(this));
            ExecuteSbtCommand = new RelayCommand(_ => Operations.SbtInvocation.Execute(this));
            GenerateIdfCommand = new RelayCommand(_ => Operations.IdfGeneration.Execute(this));
            LoadMaterialsLibraryCommand = new RelayCommand(_ => Operations.MaterialsLibraryLoad.Execute(this));
            LoadIfcBuildingCommand = new RelayCommand(_ => Operations.BuildingLoad.Execute(this));
            // "UpdateOutputDirectly" is because binding the output text to a property is unusably slow
            // i haven't figured out a better workaround yet
            UpdateOutputDirectly = updateOutputDirectly;
        }

        public Action<string> UpdateOutputDirectly { get; private set; }
    }
}
