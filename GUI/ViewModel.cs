using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Windows.Input;

using IfcBuildingInformation = IfcInformationExtractor.BuildingInformation;
using IfcConstruction = ConstructionManagement.ModelConstructions.ModelConstruction;
using IfcConstructionMappingSource = ConstructionManagement.ModelConstructions.ModelMappingSource;
using IfcElement = IfcInformationExtractor.Element;

using MaterialLibraryEntry = ConstructionManagement.MaterialLibrary.LibraryEntry;

namespace GUI
{
    class ViewModel : INotifyPropertyChanged
    {

        private SbtBuildingInformation sbtBuilding;
        private IfcBuildingInformation ifcBuilding;
        private ICollection<MaterialLibraryEntry> libraryMaterials;
        private ObservableCollection<IfcConstructionMappingSource> ifcConstructionMappingSources;
        private readonly IddManager idds = new IddManager();

        readonly private OperationInformation sbCalculation;
        readonly private OperationInformation buildingLoad;
        readonly private OperationInformation materialLibraryLoad;
        readonly private OperationInformation idfGeneration;

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
                Updated("CurrentSbtBuilding");
            }
        }

        public IfcBuildingInformation CurrentIfcBuilding
        {
            get { return ifcBuilding; }
            set
            {
                ifcBuilding = value;
                IfcConstructionMappingSources = ifcBuilding != null ? new ObservableCollection<IfcConstructionMappingSource>(ifcBuilding.ConstructionMappingSources) : null;
                PerformAutomaticMaterialMapping();
                Updated("CurrentIfcBuilding");
            }
        }

        public ICollection<MaterialLibraryEntry> LibraryMaterials
        {
            get { return libraryMaterials; }
            set
            {
                libraryMaterials = value;
                PerformAutomaticMaterialMapping();
                Updated("LibraryMaterials");
            }
        }

        public ObservableCollection<IfcConstructionMappingSource> IfcConstructionMappingSources
        {
            get { return ifcConstructionMappingSources; }
            set
            {
                ifcConstructionMappingSources = value;
                Updated("IfcConstructionMappingSources");
            }
        }

        public string InputIfcFilePath
        {
            get { return Properties.Settings.Default.InputIfcFilename; }
            set
            {
                Properties.Settings.Default.InputIfcFilename = value;
                try
                {
                    OutputIfcFilePath = System.IO.Path.Combine(System.IO.Path.GetDirectoryName(value), System.IO.Path.GetFileNameWithoutExtension(value) + "-SB.ifc");
                }
                catch (Exception) { }
                try
                {
                    OutputIdfFilePath = System.IO.Path.ChangeExtension(value, "idf");
                }
                catch (Exception) { }
                Updated("InputIfcFilePath");
            }
        }

        public string OutputIfcFilePath
        {
            get { return Properties.Settings.Default.OutputIfcFilename; }
            set
            {
                Properties.Settings.Default.OutputIfcFilename = value;
                Updated("OutputIfcFilePath");
            }
        }

        public string OutputIdfFilePath
        {
            get { return Properties.Settings.Default.OutputIdfFilename; }
            set
            {
                Properties.Settings.Default.OutputIdfFilename = value;
                Updated("OutputIdfFilePath");
            }
        }

        public string MaterialsLibraryPath
        {
            get { return Properties.Settings.Default.MaterialsLibraryFilename; }
            set
            {
                Properties.Settings.Default.MaterialsLibraryFilename = value;
                Updated("MaterialsLibraryPath");
            }
        }

        public bool WriteIfc
        {
            get { return Properties.Settings.Default.CreateOutputIfc; }
            set
            {
                Properties.Settings.Default.CreateOutputIfc = value;
                Updated("WriteIfc");
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
            set { Properties.Settings.Default.EnergyPlusVersionIndexToWrite = value; }
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
                Updated("BuildingLocation");
            }
        }
        public double TimeZone
        {
            get { return Properties.Settings.Default.TimeZone; }
            set { Properties.Settings.Default.TimeZone = value; }
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
                Updated("StartMonth");
            }
        }
        public int StartDay
        {
            get { return Properties.Settings.Default.StartDay; }
            set
            { 
                Properties.Settings.Default.StartDay = value;
                Updated("StartDay");
            }
        }
        public int EndMonth
        {
            get { return Properties.Settings.Default.EndMonth; }
            set
            {
                Properties.Settings.Default.EndMonth = value;
                Updated("EndMonth");
            }
        }
        public int EndDay
        {
            get { return Properties.Settings.Default.EndDay; }
            set
            {
                Properties.Settings.Default.EndDay = value;
                Updated("EndDay");
            }
        }
        public int Timestep
        {
            get { return Properties.Settings.Default.Timestep; }
            set { Properties.Settings.Default.Timestep = value; }
        }

        public bool CurrentlyCalculatingSBs { get { return this.sbCalculation.InProgress; } }
        public bool CurrentlyLoadingMaterialLibrary { get { return this.materialLibraryLoad.InProgress; } }
        public bool CurrentlyLoadingIfcModel { get { return this.buildingLoad.InProgress; } }
        public bool CurrentlyGeneratingIdf { get { return this.idfGeneration.InProgress; } }

        public string ReasonForDisabledSBCalculation
        {
            get
            {
                List<string> reasons = new List<string>();
                if (CurrentlyCalculatingSBs) { reasons.Add("Space boundaries are already being calculated."); }
                if (CurrentlyLoadingIfcModel) { reasons.Add("The IFC model constructions are being loaded (these operations cannot be simultaneous)."); }
                if (String.IsNullOrWhiteSpace(InputIfcFilePath) || !System.IO.File.Exists(InputIfcFilePath)) { reasons.Add("The specified IFC file does not exist."); }
                return reasons.Count == 0 ? null : "Space boundaries cannot be calculated:\n" + String.Join(Environment.NewLine, reasons);
            }
        }

        public string ReasonForDisabledMaterialLibraryLoad
        {
            get
            {
                List<string> reasons = new List<string>();
                if (CurrentlyLoadingMaterialLibrary) { reasons.Add("A material library is already being loaded."); }
                if (String.IsNullOrWhiteSpace(MaterialsLibraryPath) || !System.IO.File.Exists(MaterialsLibraryPath)) { reasons.Add("The specified material library file does not exist."); }
                return reasons.Count == 0 ? null : "The material library cannot be loaded:\n" + String.Join(Environment.NewLine, reasons);
            }
        }

        public string ReasonForDisabledIfcModelLoad
        {
            get
            {
                List<string> reasons = new List<string>();
                if (CurrentlyLoadingIfcModel) { reasons.Add("IFC constructions are already being loaded."); }
                if (String.IsNullOrWhiteSpace(InputIfcFilePath) || !System.IO.File.Exists(InputIfcFilePath)) { reasons.Add("The specified IFC file does not exist."); }
                if (CurrentlyCalculatingSBs) { reasons.Add("Space boundaries are currently being calculated (these operations cannot be simultaneous)."); }
                return reasons.Count == 0 ? null : "IFC constructions cannot be loaded:\n" + String.Join(Environment.NewLine, reasons);
            }
        }

        public string ReasonForDisabledIdfGeneration
        {
            get
            {
                List<string> reasons = new List<string>();
                if (CurrentlyGeneratingIdf) { reasons.Add("An IDF is currently being generated."); }
                if (CurrentSbtBuilding == null) { reasons.Add("No space boundaries have been loaded for this model."); }
                if (CurrentIfcBuilding == null) { reasons.Add("No constructions have been loaded for this model."); }
                if (String.IsNullOrWhiteSpace(OutputIdfFilePath)) { reasons.Add("No output filename has been specified."); }
                if (String.IsNullOrWhiteSpace(BuildingLocation)) { reasons.Add("No building location has been specified."); }
                if (StartDay < 1 || StartDay > 31) { reasons.Add("The run period start day has not been set."); }
                if (StartMonth < 1 || StartMonth > 12) { reasons.Add("The run period start month has not been set."); }
                if (EndDay < 1 || EndDay > 31) { reasons.Add("The run period end day has not been set."); }
                if (EndMonth < 1 || EndMonth > 12) { reasons.Add("The run period end month has not been set."); }
                try { if (new DateTime(2012, EndMonth, EndDay) < new DateTime(2012, StartMonth, StartDay)) { reasons.Add("The run period end date is prior to the run period start date."); } } catch (Exception) { }
                return reasons.Count == 0 ? null : "An IDF cannot be generated:\n" + String.Join(Environment.NewLine, reasons);
            }
        }

        public bool AttachDebuggerPriorToIdfGeneration { get; set; }
        public string SbSpaceFilter { get; set; }
        public string SbElementFilter { get; set; }
        public string Flags { get; set; }

        public IddManager Idds { get { return idds; } }

        public ViewModel(Action<string> updateOutputDirectly)
        {
            this.sbCalculation = new OperationInformation(Operations.SbtInvocation.Execute, this, () => PropertyChanged(this, new PropertyChangedEventArgs("CurrentlyCalculatingSBs")));
            this.materialLibraryLoad = new OperationInformation(Operations.MaterialsLibraryLoad.Execute, this, () => PropertyChanged(this, new PropertyChangedEventArgs("CurrentlyLoadingMaterialLibrary")));
            this.buildingLoad = new OperationInformation(Operations.BuildingLoad.Execute, this, () => PropertyChanged(this, new PropertyChangedEventArgs("CurrentlyLoadingIfcModel")));
            this.idfGeneration = new OperationInformation(Operations.IdfGeneration.Execute, this, () => PropertyChanged(this, new PropertyChangedEventArgs("CurrentlyGeneratingIDF")));

            var propertyDependencies = new[] 
            {
                new
                {
                    Dependent = "ReasonForDisabledSBCalculation",
                    DependentOn = new[] { "CurrentlyCalculatingSBs", "CurrentlyLoadingIfcModel", "InputIfcFilePath" }
                },
                new
                {
                    Dependent = "ReasonForDisabledMaterialLibraryLoad",
                    DependentOn = new[] { "CurrentlyLoadingMaterialLibrary", "MaterialsLibraryPath" }
                },
                new
                {
                    Dependent = "ReasonForDisabledIfcModelLoad",
                    DependentOn = new[] { "CurrentlyLoadingIfcModel", "InputIfcFilePath", "CurrentlyCalculatingSBs" }
                },
                new 
                { 
                    Dependent = "ReasonForDisabledIdfGeneration", 
                    DependentOn = new[] 
                    { 
                        "CurrentlyGeneratingIdf",
                        "CurrentSbtBuilding",
                        "CurrentIfcBuilding",
                        "OutputIdfFilePath",
                        "BuildingLocation",
                        "StartDay",
                        "StartMonth",
                        "EndDay",
                        "EndMonth"
                    }
                },
                new { Dependent = "AvailableStartDays", DependentOn = new[] { "StartMonth" } },
                new { Dependent = "AvailableEndDays", DependentOn = new[] { "EndMonth" } }
            };

            this.PropertyChanged += (_, args) =>
            {
                foreach (var d in propertyDependencies)
                {
                    if (d.DependentOn.Contains(args.PropertyName)) { PropertyChanged(this, new PropertyChangedEventArgs(d.Dependent)); }
                }
                CommandManager.InvalidateRequerySuggested();
            };

            BrowseToInputIfcFileCommand = new RelayCommand(_ => Operations.Miscellaneous.BrowseToInputIfcFile(this));
            BrowseToOutputIfcFileCommand = new RelayCommand(_ => Operations.Miscellaneous.BrowseToOutputIfcFile(this), _ => this.WriteIfc);
            BrowseToOutputIdfFileCommand = new RelayCommand(_ => Operations.Miscellaneous.BrowseToOutputIdfFile(this));
            BrowseToMaterialsLibraryCommand = new RelayCommand(_ => Operations.Miscellaneous.BrowseToMaterialsLibrary(this));
            ExecuteSbtCommand = new RelayCommand(_ => sbCalculation.Operate(), _ => ReasonForDisabledSBCalculation == null);
            GenerateIdfCommand = new RelayCommand(_ => idfGeneration.Operate(), _ => ReasonForDisabledIdfGeneration == null);
            LoadMaterialsLibraryCommand = new RelayCommand(_ => materialLibraryLoad.Operate(), _ => ReasonForDisabledMaterialLibraryLoad == null);
            LoadIfcBuildingCommand = new RelayCommand(_ => buildingLoad.Operate(), _ => ReasonForDisabledIfcModelLoad == null);
            LinkConstructionsCommand = new RelayCommand(
                obj =>
                {
                    IEnumerable<object> selectedIfcConstructions = obj as IEnumerable<object>;
                    if (selectedIfcConstructions != null)
                    {
                        Operations.Miscellaneous.LinkConstructions(this.SelectedIdfConstruction, selectedIfcConstructions.Select(c => c as IfcConstructionMappingSource));
                    }
                },
                obj =>
                {
                    IEnumerable<object> selectedIfcConstructions = obj as IEnumerable<object>;
                    if (this.SelectedIdfConstruction != null && selectedIfcConstructions != null)
                    {
                        return selectedIfcConstructions.All(c => c is IfcConstructionMappingSource && ((IfcConstructionMappingSource)c).IsForWindows == this.SelectedIdfConstruction.IsForWindows);
                    }
                    return false;
                });
            ViewIdfCommand = new RelayCommand(_ => Operations.Miscellaneous.ViewIdf(this.OutputIdfFilePath), _ => System.IO.File.Exists(this.OutputIdfFilePath));
            
            // "UpdateOutputDirectly" is because binding the output text to a property is unusably slow
            // i haven't figured out a better workaround yet
            UpdateOutputDirectly = updateOutputDirectly;
        }

        public Action<string> UpdateOutputDirectly { get; private set; }

        private void Updated(string propertyName)
        {
            if (PropertyChanged != null) { PropertyChanged(this, new PropertyChangedEventArgs(propertyName)); }
        }

        private void PerformAutomaticMaterialMapping()
        {
            if (this.CurrentIfcBuilding != null && this.LibraryMaterials != null)
            {
                Func<string, string> removeSpaces = str => str.Replace(" ", String.Empty);
                Dictionary<string, MaterialLibraryEntry> library = new Dictionary<string, MaterialLibraryEntry>();
                foreach (MaterialLibraryEntry entry in this.LibraryMaterials)
                {
                    library[removeSpaces(entry.Name)] = entry;
                }
                foreach (IfcConstructionMappingSource src in this.CurrentIfcBuilding.ConstructionMappingSources)
                {
                    MaterialLibraryEntry match;
                    if (library.TryGetValue(removeSpaces(src.Name), out match))
                    {
                        src.MappingTarget = match;
                    }
                }
            }
        }

        // there's a DateTime method I could use here but I don't want to be too much smarter than E+
        static private IEnumerable<int> AvailableDaysForMonth(int month)
        {
            return Enumerable.Range(1,
                month == 2 ? 29 :
                month == 4 ? 30 :
                month == 6 ? 30 :
                month == 9 ? 30 :
                month == 11 ? 30 : 31);
        }
    }
}
