using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;

using IfcSpace = IfcInformationExtractor.Space;

namespace GUI.Operations
{
    static partial class IdfGeneration
    {
        static public void Execute(ViewModel vm)
        {
            if (!vm.CurrentlyGeneratingIdf)
            {
                try
                {
                    if (vm.CurrentIfcBuilding == null)
                    {
                        System.Windows.MessageBox.Show(
                            "IFC constructions and materials have not been loaded. Please load and assign library mappings to the IFC model's constructions and materials in the \"Constructions & Materials\" tab before attempting to generate an IDF.",
                            "Cannot generate IDF",
                            System.Windows.MessageBoxButton.OK,
                            System.Windows.MessageBoxImage.Error);
                        return;
                    }

                    if (vm.CurrentSbtBuilding == null)
                    {
                        System.Windows.MessageBox.Show(
                            "Space boundaries have not been loaded. Please load space boundaries in the \"Space Boundaries\" tab before attempting to generate an IDF.",
                            "Cannot generate IDF",
                            System.Windows.MessageBoxButton.OK,
                            System.Windows.MessageBoxImage.Error);
                        return;
                    }

                    if (vm.CurrentSbtBuilding.IfcFilename != vm.CurrentIfcBuilding.Filename)
                    {
                        System.Windows.MessageBox.Show(
                            "The filename that space boundaries have been loaded for is not the same filename for the constructions and materials mapping. Either re-load space boundaries or re-load the model's constructions and materials.",
                            "Cannot generate IDF",
                            System.Windows.MessageBoxButton.OK,
                            System.Windows.MessageBoxImage.Error);
                        return;
                    }

                    if (vm.IfcConstructions.Any(c => c.ParticipatesInSpaceBoundary.HasValue && c.ParticipatesInSpaceBoundary.Value && c.IdfMappingTarget == null)) {
                        System.Windows.MessageBox.Show(
                            "There are some IFC constructions that have not been mapped to material library entries. Please ensure that all IFC constructions are mapped in the \"Constructions & Materials\" tab.",
                            "Cannot generate IDF",
                            System.Windows.MessageBoxButton.OK,
                            System.Windows.MessageBoxImage.Error);
                        return;
                    }

                    BackgroundWorker worker = new BackgroundWorker();
                    worker.WorkerReportsProgress = true;
                    worker.DoWork += new DoWorkEventHandler(DoIdfGenerationWork);
                    worker.ProgressChanged += new ProgressChangedEventHandler((sender, e) =>
                    {
                        string msg = e.UserState as string;
                        if (msg != null) { vm.UpdateOutputDirectly(msg); }
                    });
                    worker.RunWorkerCompleted += new RunWorkerCompletedEventHandler((sender, e) =>
                    {
                        vm.CurrentlyGeneratingIdf = false;
                    });

                    Parameters p = new Parameters();

                    p.OutputFilename = vm.OutputIdfFilePath;
                    p.LocationName = vm.BuildingLocation;
                    p.TimeZone = vm.TimeZone;
                    p.SolarDistribution = vm.SolarDistribution;
                    p.LoadsConvergenceTolerance = vm.LoadsConvergenceTolerance;
                    p.TemperatureConvergenceTolerance = vm.TemperatureConvergenceTolerance;

                    p.StartMonth = vm.StartMonth;
                    p.StartDay = vm.StartDay;
                    p.EndMonth = vm.EndMonth;
                    p.EndDay = vm.EndDay;

                    p.Timestep = vm.Timestep;

                    p.SbtBuilding = vm.CurrentSbtBuilding;
                    p.IfcBuilding = vm.CurrentIfcBuilding;

                    p.IfcConstructionsByName = new Dictionary<string, IfcConstruction>();
                    foreach (IfcConstruction c in vm.IfcConstructions)
                    {
                        p.IfcConstructionsByName[c.Name] = c;
                    }
                    p.GetIdd = () => vm.Idds.GetIddFor((EnergyPlusVersion)vm.EnergyPlusVersionIndexToWrite, msg => worker.ReportProgress(0, msg + Environment.NewLine));
                    p.Notify = msg => worker.ReportProgress(0, msg);

                    p.AttachDebugger = vm.AttachDebuggerPriorToIdfGeneration;

                    vm.CurrentlyGeneratingIdf = true;
                    worker.RunWorkerAsync(p);
                }
                catch (Exception)
                {
                    vm.CurrentlyGeneratingIdf = false;
                }
            }
        }

        static private void DoIdfGenerationWork(object sender, DoWorkEventArgs e)
        {
            Parameters p = e.Argument as Parameters;
            if (p != null)
            {
                if (p.AttachDebugger) { System.Diagnostics.Debugger.Launch(); }
                try
                {
                    p.Notify("Getting IDD.\n");
                    LibIdf.Idd.Idd idd = p.GetIdd();
                    p.Notify("Got IDD.\n");

                    ConstructionManager.Manager constructionManager = new ConstructionManager.Manager(id =>
                    {
                        if (id > p.SbtBuilding.Elements.Count) { return null; }
                        string elementGuid = p.SbtBuilding.Elements[id - 1].Guid;
                        IfcInformationExtractor.Element ifcElement;
                        if (!p.IfcBuilding.ElementsByGuid.TryGetValue(elementGuid, out ifcElement)) { return null; }
                        IfcConstruction c;
                        return p.IfcConstructionsByName.TryGetValue(ifcElement.AssociatedConstruction.Name, out c) ? c.IdfMappingTarget : null;
                    });

                    IDictionary<string, string> zoneNamesByGuid = GatherZoneNamesByGuid(FindUsedSpaces(p.SbtBuilding.SpaceBoundaries, p.IfcBuilding.SpacesByGuid));

                    IDictionary<string, BuildingSurface> surfacesByGuid = new Dictionary<string, BuildingSurface>();
                    foreach (Sbt.CoreTypes.SpaceBoundary sb in p.SbtBuilding.SpaceBoundaries.Where(sb => sb.IsVirtual || !sb.Element.IsFenestration))
                    {
                        if (sb.Level == 2)
                        {
                            surfacesByGuid[sb.Guid] = new BuildingSurface(
                                sb,
                                constructionManager.ConstructionNameForLayers(sb.MaterialLayers),
                                zoneNamesByGuid[sb.BoundedSpace.Guid]);
                        }
                        else
                        {
                            surfacesByGuid[sb.Guid] = new BuildingSurface(
                                sb,
                                constructionManager.ConstructionNameForSurface(sb.Element.MaterialId),
                                zoneNamesByGuid[sb.BoundedSpace.Guid]);
                        }
                    }

                    IList<FenestrationSurface> fenestrations = new List<FenestrationSurface>(p.SbtBuilding.SpaceBoundaries
                        .Where(sb => !sb.IsVirtual && sb.Element.IsFenestration && sb.ContainingBoundary != null) // all fenestration sbs *should* have a containing boundary, but...
                        .Select(sb => new FenestrationSurface(sb, surfacesByGuid[sb.ContainingBoundary.Guid], constructionManager.ConstructionNameForLayers(sb.MaterialLayers))));

                    IDictionary<string, Sbt.CoreTypes.Solid> elementGeometriesByGuid = new Dictionary<string, Sbt.CoreTypes.Solid>();
                    foreach (Sbt.CoreTypes.ElementInfo element in p.SbtBuilding.Elements)
                    {
                        elementGeometriesByGuid[element.Guid] = element.Geometry;
                    }
                    var shadings =
                        p.IfcBuilding.ElementsByGuid.Values
                        .Where(element => element.IsShading)
                        .Select(element => new Shading(element.Guid, elementGeometriesByGuid[element.Guid], null));

                    IdfCreator creator = IdfCreator.Build(p.EPVersion, idd, p.Notify);

                    creator.AddConstantContents();
                    creator.AddLocation(p.LocationName, p.TimeZone, p.IfcBuilding.Latitude, p.IfcBuilding.Longitude, p.IfcBuilding.Elevation);
                    creator.AddBuilding(p.NorthAxis, p.LoadsConvergenceTolerance, p.TemperatureConvergenceTolerance, p.SolarDistribution, p.BuildingTerrain);
                    creator.AddTimestep(p.Timestep);
                    creator.AddRunPeriod(p.StartMonth, p.StartDay, p.EndMonth, p.EndDay);
                    foreach (KeyValuePair<string, string> zone in zoneNamesByGuid) { creator.AddZone(zone.Value, zone.Key); }
                    foreach (BuildingSurface surf in surfacesByGuid.Values) { creator.AddBuildingSurface(surf); }
                    foreach (FenestrationSurface fen in fenestrations) { creator.AddFenestration(fen); }
                    foreach (Shading s in shadings) { creator.AddShading(s); }
                    foreach (ConstructionManager.Construction c in constructionManager.AllConstructions) { creator.AddConstruction(c); }
                    foreach (ConstructionManager.OutputLayer m in constructionManager.AllMaterials) { creator.AddMaterial(m); }

                    creator.WriteToFile(p.OutputFilename);
                    p.Notify("IDF written.\n");
                }
                catch (Exception ex)
                {
                    p.Notify("Operation failed: " + ex.Message + Environment.NewLine);
                }
            }
        }

        static private ICollection<IfcSpace> FindUsedSpaces(IEnumerable<Sbt.CoreTypes.SpaceBoundary> sbs, IDictionary<string, IfcSpace> spaces)
        {
            HashSet<string> usedGuids = new HashSet<string>(sbs.Select(sb => sb.BoundedSpace.Guid));
            return new List<IfcSpace>(usedGuids.Select(guid => spaces[guid]));
        }

        static private IDictionary<string, string> GatherZoneNamesByGuid(ICollection<IfcSpace> usedSpaces)
        {
            IDictionary<string, string> res = new Dictionary<string, string>();

            bool allLongNamesPresent = !usedSpaces.Any(s => String.IsNullOrWhiteSpace(s.LongName));
            if (allLongNamesPresent)
            {
                if (usedSpaces.Select(s => s.LongName).Distinct().Count() == usedSpaces.Count)
                {
                    foreach (IfcSpace s in usedSpaces) { res[s.Guid] = s.LongName; }
                    return res;
                }
            }

            bool allNamesPresent = !usedSpaces.Any(s => String.IsNullOrWhiteSpace(s.Name));
            if (allNamesPresent)
            {
                if (usedSpaces.Select(s => s.Name).Distinct().Count() == usedSpaces.Count)
                {
                    foreach (IfcSpace s in usedSpaces) { res[s.Guid] = s.Name; }
                }
            }

            if (allNamesPresent && allLongNamesPresent)
            {
                // all names and long names present but not independently unique
                var combined = usedSpaces.Select(s => String.Format("{0} {1}", s.LongName, s.Name));
                if (combined.Distinct().Count() == usedSpaces.Count)
                {
                    foreach (IfcSpace s in usedSpaces) { res[s.Guid] = String.Format("{0} {1}", s.LongName, s.Name); }
                    return res;
                }
            }

            // fall back to guids
            // TODO: check for case-sensitivity collisions here (ugh revit)
            foreach (IfcSpace s in usedSpaces) { res[s.Guid] = s.Guid; }
            return res;
        }
    }
}
