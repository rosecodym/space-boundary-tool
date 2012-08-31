﻿using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Windows.Input;

using IfcBuildingInformation = IfcInformationExtractor.BuildingInformation;
using IfcConstruction = IfcInformationExtractor.Construction;
using IfcElement = IfcInformationExtractor.Element;

using MaterialLibraryEntry = MaterialLibrary.LibraryEntry;

namespace GUI
{
    class ViewModel : INotifyPropertyChanged
    {

        private SbtBuildingInformation sbtBuilding;
        private IfcBuildingInformation ifcBuilding;
        private ICollection<MaterialLibraryEntry> libraryMaterials;
        private ObservableCollection<IfcConstruction> ifcConstructions;
        private readonly IddManager idds = new IddManager();

        private bool calculatingSBs = false;
        private bool loadingMaterialLibrary = false;
        private bool loadingIfcModel = false;
        private bool generatingIdf = false;

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
                Updated("CurrentSbtBuilding");
            }
        }

        public IfcBuildingInformation CurrentIfcBuilding
        {
            get { return ifcBuilding; }
            set
            {
                ifcBuilding = value;
                IfcConstructions = new ObservableCollection<IfcConstruction>(ifcBuilding.Constructions.Select(c => new IfcConstruction(c)));
                Updated("CurrentIfcBuilding");
            }
        }

        public ICollection<MaterialLibraryEntry> LibraryMaterials
        {
            get { return libraryMaterials; }
            set
            {
                libraryMaterials = value;
                Updated("LibraryMaterials");
            }
        }

        public ObservableCollection<IfcConstruction> IfcConstructions
        {
            get { return ifcConstructions; }
            set
            {
                ifcConstructions = value;
                CheckIfcConstructionsForSbParticipation();
                Updated("IfcConstructions");
            }
        }

        public string InputIfcFilePath
        {
            get { return Properties.Settings.Default.InputIfcFilename; }
            set
            {
                Properties.Settings.Default.InputIfcFilename = value;
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

        public bool CurrentlyCalculatingSBs
        {
            get { return calculatingSBs; }
            set
            {
                calculatingSBs = value;
            }
        }
        public bool CurrentlyLoadingMaterialLibrary
        {
            get { return loadingMaterialLibrary; }
            set
            {
                loadingMaterialLibrary = value;
            }
        }
        public bool CurrentlyLoadingIfcModel
        {
            get { return loadingIfcModel; }
            set
            {
                loadingIfcModel = value;
            }
        }
        public bool CurrentlyGeneratingIdf
        {
            get { return generatingIdf; }
            set
            {
                generatingIdf = value;
                Updated("CurrentlyGeneratingIdf");
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

        public IddManager Idds { get { return idds; } }

        public ViewModel(Action<string> updateOutputDirectly)
        {
            var propertyDependencies = new[] 
            {
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
            };

            BrowseToInputIfcFileCommand = new RelayCommand(_ => Operations.Miscellaneous.BrowseToInputIfcFile(this));
            BrowseToOutputIfcFileCommand = new RelayCommand(_ => Operations.Miscellaneous.BrowseToOutputIfcFile(this), _ => this.WriteIfc);
            BrowseToOutputIdfFileCommand = new RelayCommand(_ => Operations.Miscellaneous.BrowseToOutputIdfFile(this));
            BrowseToMaterialsLibraryCommand = new RelayCommand(_ => Operations.Miscellaneous.BrowseToMaterialsLibrary(this));
            ExecuteSbtCommand = new RelayCommand(_ => Operations.SbtInvocation.Execute(this), _ => !CurrentlyCalculatingSBs && !CurrentlyLoadingIfcModel);
            GenerateIdfCommand = new RelayCommand(_ => Operations.IdfGeneration.Execute(this), _ => ReasonForDisabledIdfGeneration == null);
            LoadMaterialsLibraryCommand = new RelayCommand(_ => Operations.MaterialsLibraryLoad.Execute(this), _ => !CurrentlyLoadingMaterialLibrary && !String.IsNullOrWhiteSpace(this.MaterialsLibraryPath));
            LoadIfcBuildingCommand = new RelayCommand(_ => Operations.BuildingLoad.Execute(this), _ => !CurrentlyCalculatingSBs && !CurrentlyLoadingIfcModel);
            LinkConstructionsCommand = new RelayCommand(
                obj =>
                {
                    IEnumerable<object> selectedIfcConstructions = obj as IEnumerable<object>;
                    if (selectedIfcConstructions != null)
                    {
                        Operations.Miscellaneous.LinkConstructions(this.SelectedIdfConstruction, selectedIfcConstructions.Select(c => c as IfcConstruction));
                    }
                },
                obj =>
                {
                    IEnumerable<object> selectedIfcConstructions = obj as IEnumerable<object>;
                    if (this.SelectedIdfConstruction != null && selectedIfcConstructions != null)
                    {
                        return selectedIfcConstructions.All(c => c is IfcConstruction && ((IfcConstruction)c).IsForWindows == this.SelectedIdfConstruction.IsForWindows);
                    }
                    return false;
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

        private void Updated(string propertyName)
        {
            if (PropertyChanged != null) { PropertyChanged(this, new PropertyChangedEventArgs(propertyName)); }
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
