using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Windows.Input;

using IfcBuildingInformation = IfcInformationExtractor.BuildingInformation;
using IfcConstruction = IfcInformationExtractor.Construction;
using IfcElement = IfcInformationExtractor.Element;

using MaterialLibraryEntry = GUI.Materials.LibraryEntries.Opaque;

namespace GUI
{
    class ViewModel : INotifyPropertyChanged
    {

        private SbtBuildingInformation sbtBuilding;
        private IfcBuildingInformation ifcBuilding;
        private ICollection<MaterialLibraryEntry> libraryMaterials;
        private ObservableCollection<IfcConstruction> ifcConstructions;
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
        public ICommand LinkConstructionsCommand { get; private set; }
        public ICommand GenerateIdfCommand { get; private set; }
        public ICommand ViewIdfCommand { get; private set; }

        public SbtBuildingInformation CurrentSbtBuilding
        {
            get { return sbtBuilding; }
            set
            {
                sbtBuilding = value;
                CheckIfcConstructionsForSbParticipation();
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
                IfcConstructions = new ObservableCollection<IfcConstruction>(ifcBuilding.Constructions.Select(c => new IfcConstruction(c)));
                if (PropertyChanged != null)
                {
                    PropertyChanged(this, new PropertyChangedEventArgs("CurrentIfcBuilding"));
                }
            }
        }

        public ICollection<MaterialLibraryEntry> LibraryMaterials
        {
            get { return libraryMaterials; }
            set
            {
                libraryMaterials = value;
                if (PropertyChanged != null) { PropertyChanged(this, new PropertyChangedEventArgs("LibraryMaterials")); }
            }
        }

        public ObservableCollection<IfcConstruction> IfcConstructions
        {
            get { return ifcConstructions; }
            set
            {
                ifcConstructions = value;
                CheckIfcConstructionsForSbParticipation();
                if (PropertyChanged != null) { PropertyChanged(this, new PropertyChangedEventArgs("IfcConstructions")); }
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
                if (PropertyChanged != null) { 
                    PropertyChanged(this, new PropertyChangedEventArgs("MaterialsLibraryPath"));
                    PropertyChanged(this, new PropertyChangedEventArgs("MaterialsLibraryLoadable"));
                }
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

        public Operations.IdfGeneration.SolarDistribution[] AvailableSolarDistributions
        {
            get
            {
                return new Operations.IdfGeneration.SolarDistribution[] { 
                    Operations.IdfGeneration.SolarDistribution.MinimalShadowing,
                    Operations.IdfGeneration.SolarDistribution.FullExterior, 
                    Operations.IdfGeneration.SolarDistribution.FullExteriorWithReflections, 
                    Operations.IdfGeneration.SolarDistribution.FullInteriorAndExterior, 
                    Operations.IdfGeneration.SolarDistribution.FullInteriorAndExteriorWithReflections
                };
            }
        }
        public Operations.IdfGeneration.BuildingTerrain[] AvailableBuildingTerrains
        {
            get
            {
                return new Operations.IdfGeneration.BuildingTerrain[] {
                    Operations.IdfGeneration.BuildingTerrain.Country,
                    Operations.IdfGeneration.BuildingTerrain.Suburbs,
                    Operations.IdfGeneration.BuildingTerrain.City,
                    Operations.IdfGeneration.BuildingTerrain.Ocean,
                    Operations.IdfGeneration.BuildingTerrain.Urban
                };
            }
        }
        public IEnumerable<int> AvailableMonths
        {
            get { return Enumerable.Range(1, 12); } 
        }
        public IEnumerable<int> AvailableStartDays { get { return AvailableDaysForMonth(StartMonth); } }
        public IEnumerable<int> AvailableEndDays { get { return AvailableDaysForMonth(EndMonth); } }
        public int[] AvailableTimesteps { get { return new int[] { 1, 2, 3, 4, 5, 6, 10, 12, 15, 20, 30, 60 }; } }

        public int EnergyPlusVersionIndexToWrite
        {
            get { return Properties.Settings.Default.EnergyPlusVersionIndexToWrite; }
            set
            {
                Properties.Settings.Default.EnergyPlusVersionIndexToWrite = value;
                if (PropertyChanged != null) { PropertyChanged(this, new PropertyChangedEventArgs("EnergyPlusVersionIndexToWrite")); }
            }
        }

        public MaterialLibraryEntry SelectedIdfConstruction { get; set; }

        public string BuildingLocation
        {
            get
            {
                return Properties.Settings.Default.BuildingLocation;
            }
            set
            {
                Properties.Settings.Default.BuildingLocation = value;
                if (PropertyChanged != null) { PropertyChanged(this, new PropertyChangedEventArgs("BuildingLocation")); }
            }
        }
        public double TimeZone
        {
            get
            {
                return Properties.Settings.Default.TimeZone;
            }
            set
            {
                Properties.Settings.Default.TimeZone = value;
                if (PropertyChanged != null) { PropertyChanged(this, new PropertyChangedEventArgs("TimeZone")); }
            }
        }
        public Operations.IdfGeneration.SolarDistribution SolarDistribution
        {
            get
            {
                Operations.IdfGeneration.SolarDistribution res;
                return Enum.TryParse<Operations.IdfGeneration.SolarDistribution>(Properties.Settings.Default.SolarDistribution, out res) ? res : Operations.IdfGeneration.SolarDistribution.FullExterior;
            }
            set { Properties.Settings.Default.SolarDistribution = value.ToString(); }
        }
        public Operations.IdfGeneration.BuildingTerrain BuildingTerrain
        {
            get
            {
                Operations.IdfGeneration.BuildingTerrain res;
                return Enum.TryParse<Operations.IdfGeneration.BuildingTerrain>(Properties.Settings.Default.BuildingTerrain, out res) ? res : Operations.IdfGeneration.BuildingTerrain.Suburbs;
            }
            set { Properties.Settings.Default.BuildingTerrain = value.ToString(); }
        }
        public double LoadsConvergenceTolerance
        {
            get { return Properties.Settings.Default.LoadsConverganceTolerance; }
            set { Properties.Settings.Default.LoadsConverganceTolerance = value; }
        }
        public double TemperatureConvergenceTolerance
        {
            get { return Properties.Settings.Default.TemperatureConvergenceTolerance; }
            set { Properties.Settings.Default.TemperatureConvergenceTolerance = value; }
        }
        public int StartMonth
        {
            get { return Properties.Settings.Default.StartMonth; }
            set
            {
                Properties.Settings.Default.StartMonth = value;
                if (PropertyChanged != null)
                {
                    PropertyChanged(this, new PropertyChangedEventArgs("StartMonth"));
                    PropertyChanged(this, new PropertyChangedEventArgs("AvailableStartDays"));
                }
            }
        }
        public int StartDay
        {
            get { return Properties.Settings.Default.StartDay; }
            set { Properties.Settings.Default.StartDay = value; }
        }
        public int EndMonth
        {
            get { return Properties.Settings.Default.EndMonth; }
            set
            {
                Properties.Settings.Default.EndMonth = value;
                if (PropertyChanged != null)
                {
                    PropertyChanged(this, new PropertyChangedEventArgs("EndMonth"));
                    PropertyChanged(this, new PropertyChangedEventArgs("AvailableEndDays"));
                }
            }
        }
        public int EndDay
        {
            get { return Properties.Settings.Default.EndDay; }
            set { Properties.Settings.Default.EndDay = value; }
        }
        public int Timestep
        {
            get { return Properties.Settings.Default.Timestep; }
            set { Properties.Settings.Default.Timestep = value; }
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
            get { return !Busy && !String.IsNullOrWhiteSpace(this.MaterialsLibraryPath); }
        }

        public bool IfcBuildingLoadable
        {
            get { return !Busy && InputIfcFilePath != String.Empty; }
        }

        public bool AttachDebuggerPriorToIdfGeneration { get; set; }
        public string SbSpaceFilter { get; set; }
        public string SbElementFilter { get; set; }

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
            LinkConstructionsCommand = new RelayCommand(obj =>
            {
                IEnumerable<object> selectedIfcConstructions = obj as IEnumerable<object>;
                if (selectedIfcConstructions != null)
                {
                    Operations.Miscellaneous.LinkConstructions(this.SelectedIdfConstruction, selectedIfcConstructions.Select(c => c as IfcConstruction));
                }
            });
            ViewIdfCommand = new RelayCommand(_ => Operations.Miscellaneous.ViewIdf(this.OutputIdfFilePath), _ => System.IO.File.Exists(this.OutputIdfFilePath));
            
            // "UpdateOutputDirectly" is because binding the output text to a property is unusably slow
            // i haven't figured out a better workaround yet
            UpdateOutputDirectly = updateOutputDirectly;
        }

        public Action<string> UpdateOutputDirectly { get; private set; }

        private void CheckIfcConstructionsForSbParticipation()
        {
            try
            {
                if (CurrentSbtBuilding != null && IfcConstructions != null)
                {
                    foreach (Sbt.CoreTypes.SpaceBoundary sb in this.CurrentSbtBuilding.SpaceBoundaries)
                    {
                        foreach (Sbt.CoreTypes.MaterialLayer layer in sb.MaterialLayers)
                        {
                            string elementGuid = this.CurrentSbtBuilding.Elements[layer.Id - 1].Guid;
                            IfcElement ifcElement = this.CurrentIfcBuilding.ElementsByGuid[elementGuid];
                            foreach (IfcConstruction c in this.IfcConstructions)
                            {
                                if (c.Name == ifcElement.AssociatedConstruction.Name)
                                {
                                    c.ParticipatesInSpaceBoundary = true;
                                }
                            }
                        }
                    }
                    foreach (IfcConstruction c in this.IfcConstructions)
                    {
                        if (!c.ParticipatesInSpaceBoundary.HasValue) { c.ParticipatesInSpaceBoundary = false; }
                    }
                }
            }
            catch (Exception) { /* no time for this right now */ }
        }

        static private IEnumerable<int> AvailableDaysForMonth(int month)
        {
            return Enumerable.Range(1,
                month == 2 ? 28 :
                month == 4 ? 30 :
                month == 6 ? 30 :
                month == 9 ? 30 :
                month == 11 ? 30 : 31);
        }
    }
}
