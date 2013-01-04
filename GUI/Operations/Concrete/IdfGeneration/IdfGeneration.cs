using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;

using Solid = Sbt.CoreTypes.Solid;
using SbtLayer = Sbt.CoreTypes.MaterialLayer;
using SbtElement = Sbt.CoreTypes.ElementInfo;
using SbtElementType = Sbt.CoreTypes.ElementType;

using ConstructionManager = ConstructionManagement.OutputManager;
using IdfConstruction = ConstructionManagement.OutputConstruction;
using IdfMaterial = ConstructionManagement.OutputLayer;

using IfcConstruction = ConstructionManagement.ModelConstructions.ModelConstruction;
using IfcElement = IfcInformationExtractor.Element;
using IfcSpace = IfcInformationExtractor.Space;

using LibraryEntry = ConstructionManagement.MaterialLibrary.LibraryEntry;

using Normal = System.Tuple<double, double, double>;

namespace GUI.Operations
{
    partial class IdfGeneration : Operation<IdfGeneration.Parameters, object> // no return
    {
        private Parameters GetParameters(ViewModel vm)
        {
            if (vm.CurrentIfcBuilding == null)
            {
                System.Windows.MessageBox.Show(
                    "IFC constructions and materials have not been loaded. Please load and assign library mappings to the IFC model's constructions and materials in the \"Constructions & Materials\" tab before attempting to generate an IDF.",
                    "Cannot generate IDF",
                    System.Windows.MessageBoxButton.OK,
                    System.Windows.MessageBoxImage.Error);
                return null;
            }
            if (vm.CurrentSbtBuilding == null)
            {
                System.Windows.MessageBox.Show(
                    "Space boundaries have not been loaded. Please load space boundaries in the \"Space Boundaries\" tab before attempting to generate an IDF.",
                    "Cannot generate IDF",
                    System.Windows.MessageBoxButton.OK,
                    System.Windows.MessageBoxImage.Error);
                return null;
            }
            if (vm.CurrentSbtBuilding.IfcFilename != vm.CurrentIfcBuilding.Filename)
            {
                System.Windows.MessageBox.Show(
                    "The filename that space boundaries have been loaded for is not the same filename for the constructions and materials mapping. Either re-load space boundaries or re-load the model's constructions and materials.",
                    "Cannot generate IDF",
                    System.Windows.MessageBoxButton.OK,
                    System.Windows.MessageBoxImage.Error);
                return null;
            }
            if (vm.IfcConstructionMappingSources.Any(c => c.MappingTarget == null))
            {
                System.Windows.MessageBox.Show(
                    "There are some IFC constructions that have not been mapped to material library entries. Please ensure that all IFC constructions are mapped in the \"Constructions & Materials\" tab.",
                    "Cannot generate IDF",
                    System.Windows.MessageBoxButton.OK,
                    System.Windows.MessageBoxImage.Error);
                return null;
            }
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
            p.MaterialIDToModelConstruction = vm.SbtMaterialIDToModelConstruction;
            p.MaterialIDToCompositeDir = vm.SbtMaterialIDToCompositeNormal;
            p.GetIdd = () => vm.Idds.GetIddFor((EnergyPlusVersion)vm.EnergyPlusVersionIndexToWrite, msg => ReportProgress(msg + Environment.NewLine));
            p.AttachDebugger = vm.AttachDebuggerPriorToIdfGeneration;
            return p;
        }

        private object GenerateIdf(Parameters p)
        {
            if (p == null) { return null; }
            if (p.AttachDebugger) { System.Diagnostics.Debugger.Launch(); }
            try
            {
                ReportProgress("Getting IDD.\n");
                LibIdf.Idd.Idd idd = p.GetIdd();
                ReportProgress("Got IDD.\n");
                ConstructionManager cmanager =
                    new ConstructionManager(
                        msg =>
                            ReportProgress(
                                msg + Environment.NewLine,
                                ProgressEvent.ProgressEventType.Warning));
                var zoneNamesByGuid = GatherZoneNamesByGuid(FindUsedSpaces(
                    p.SbtBuilding.SpaceBoundaries,
                    p.IfcBuilding.SpacesByGuid));
                IDictionary<string, BuildingSurface> surfacesByGuid =
                    new Dictionary<string, BuildingSurface>();
                var allSbs = p.SbtBuilding.SpaceBoundaries;
                var nonFen = allSbs.Where(sb =>
                {
                    if (!sb.IsVirtual) { return !sb.Element.IsFenestration; }
                    else
                    {
                        // E+ (7.1) doesn't like intrazone virtual boundaries.
                        return
                            zoneNamesByGuid[sb.BoundedSpace.Guid] !=
                            zoneNamesByGuid[sb.Opposite.BoundedSpace.Guid];
                    }
                });
                foreach (Sbt.CoreTypes.SpaceBoundary sb in nonFen)
                {
                    if (sb.Level == 2)
                    {
                        var constructions = new List<IfcConstruction>();
                        var thicknesses = new List<double>();
                        var dirs = new List<Normal>();
                        foreach (SbtLayer layer in sb.MaterialLayers)
                        {
                            var mc = p.MaterialIDToModelConstruction(layer.Id);
                            var norm = p.MaterialIDToCompositeDir(layer.Id);
                            constructions.Add(mc);
                            thicknesses.Add(layer.Thickness);
                            dirs.Add(norm);
                        }
                        surfacesByGuid[sb.Guid] = new BuildingSurface(
                            sb,
                            cmanager.ConstructionNameForLayers(
                                sb.Normal,
                                constructions,
                                dirs,
                                thicknesses),
                            zoneNamesByGuid[sb.BoundedSpace.Guid]);
                    }
                    else
                    {
                        surfacesByGuid[sb.Guid] = new BuildingSurface(
                            sb,
                            cmanager.ConstructionNameForSurface(
                                p.MaterialIDToModelConstruction(
                                    sb.Element.MaterialId)),
                            zoneNamesByGuid[sb.BoundedSpace.Guid]);
                    }
                }
                foreach (Sbt.CoreTypes.SpaceBoundary sb in nonFen)
                {
                    if (sb.Opposite != null)
                    {
                        var match = surfacesByGuid[sb.Guid];
                        var opp = surfacesByGuid[sb.Opposite.Guid];
                        match.Opposite = opp;
                    }
                }
                var fenestrations = new List<FenestrationSurface>();
                // all fenestration sbs *should* have a containing 
                // boundary, but this doesn't always happen
                var fens = allSbs.Where(sb =>
                    !sb.IsVirtual &&
                    sb.Element.IsFenestration &&
                    sb.ContainingBoundary != null);
                foreach (Sbt.CoreTypes.SpaceBoundary sb in fens)
                {
                    var constructions = new List<IfcConstruction>();
                    var thicknesses = new List<double>();
                    var dirs = new List<Normal>();
                    foreach (SbtLayer layer in sb.MaterialLayers)
                    {
                        var c = p.MaterialIDToModelConstruction(layer.Id);
                        var norm = p.MaterialIDToCompositeDir(layer.Id);
                        constructions.Add(c);
                        thicknesses.Add(layer.Thickness);
                        dirs.Add(norm);
                    }
                    fenestrations.Add(new FenestrationSurface(
                        sb,
                        surfacesByGuid[sb.ContainingBoundary.Guid],
                        cmanager.ConstructionNameForLayers(
                            sb.Normal,
                            constructions,
                            dirs,
                            thicknesses)));
                }
                var elementGeometries = new Dictionary<string, Solid>();
                foreach (SbtElement element in p.SbtBuilding.Elements)
                {
                    elementGeometries[element.Guid] = element.Geometry;
                }

                Func<BuildingSurface, bool> isWall = surf =>
                    surf.HasElementType(SbtElementType.Wall);
                var wallFilter = surfacesByGuid.Values.Where(isWall);
                var wallBoundaries = new List<BuildingSurface>(wallFilter);

                List<Shading> shadings = new List<Shading>();
                foreach (IfcElement e in p.IfcBuilding.ElementsByGuid.Values)
                {
                    // elementGeometries might not have e because it was 
                    // filtered.
                    if (e.IsShading && elementGeometries.ContainsKey(e.Guid))
                    {
                        try
                        {
                            shadings.Add(new Shading(
                                e.Guid,
                                elementGeometries[e.Guid],
                                wallBoundaries));
                        }
                        catch (Shading.ShadingConstructionFailedException ex)
                        {
                            ReportProgress(
                                "Shading generation failed: " + 
                                    ex.Message +
                                    Environment.NewLine,
                                ProgressEvent.ProgressEventType.Warning);
                        }
                    }
                }
                IdfCreator creator =
                    IdfCreator.Build(
                        p.EPVersion,
                        idd,
                        msg => ReportProgress(msg));
                creator.AddConstantContents();
                creator.AddLocation(
                    p.LocationName,
                    p.TimeZone,
                    p.IfcBuilding.Latitude,
                    p.IfcBuilding.Longitude,
                    p.IfcBuilding.Elevation);
                creator.AddBuilding(
                    p.NorthAxis,
                    p.LoadsConvergenceTolerance,
                    p.TemperatureConvergenceTolerance,
                    p.SolarDistribution,
                    p.BuildingTerrain);
                creator.AddTimestep(p.Timestep);
                creator.AddRunPeriod(
                    p.StartMonth,
                    p.StartDay,
                    p.EndMonth,
                    p.EndDay);
                foreach (string zoneName in zoneNamesByGuid.Values.Distinct())
                {
                    creator.AddZone(zoneName);
                }
                foreach (BuildingSurface surf in surfacesByGuid.Values)
                {
                    creator.AddBuildingSurface(surf);
                }
                foreach (FenestrationSurface fen in fenestrations)
                {
                    creator.AddFenestration(fen);
                }
                foreach (Shading s in shadings) { creator.AddShading(s); }
                foreach (IdfConstruction c in cmanager.AllOutputConstructions)
                {
                    creator.AddConstruction(c);
                }
                foreach (IdfMaterial m in cmanager.AllOutputLayers)
                {
                    try
                    {
                        creator.AddMaterial(m);
                    }
                    catch (Exception) {
                        ReportProgress(
                            String.Format(
                                "Material layer {0} could not be added to " +
                                "the IDF. Please report this SBT bug." +
                                Environment.NewLine,
                                m.Name),
                            ProgressEvent.ProgressEventType.Warning);
                    }
                }
                creator.WriteToFile(p.OutputFilename);
                ReportProgress("IDF written.\n");
            }
            catch (Exception ex)
            {
                ReportProgress("Operation failed: " + ex.Message + Environment.NewLine, ProgressEvent.ProgressEventType.Error);
            }
            return null;
        }

        static private ICollection<IfcSpace> FindUsedSpaces(IEnumerable<Sbt.CoreTypes.SpaceBoundary> sbs, IDictionary<string, IfcSpace> spaces)
        {
            HashSet<string> usedGuids = new HashSet<string>(sbs.Select(sb => sb.BoundedSpace.Guid));
            return new List<IfcSpace>(usedGuids.Select(guid => spaces[guid]));
        }

        static private IDictionary<string, string> GatherZoneNamesByGuid(
            ICollection<IfcSpace> usedSpaces)
        {
            IDictionary<string, string> res = new Dictionary<string, string>();

            Func<string, bool> empty = s => String.IsNullOrWhiteSpace(s);

            foreach (IfcSpace s in usedSpaces.Where(s => s.Zones.Count != 0))
            {
                var z = s.Zones.First();
                res[s.Guid] = empty(z.Name) ? z.Guid : z.Name;
            }

            var unassigned =
                new List<IfcSpace>(usedSpaces.Where(s => s.Zones.Count == 0));

            bool allLongNamesPresent = !unassigned.Any(s => empty(s.LongName));
            if (allLongNamesPresent)
            {
                var asLongNames = unassigned.Select(s => s.LongName);
                if (asLongNames.Distinct().Count() == unassigned.Count)
                {
                    foreach (IfcSpace s in unassigned)
                    { 
                        res[s.Guid] = s.LongName; 
                    }
                    return res;
                }
            }

            bool allNamesPresent = !unassigned.Any(s => empty(s.Name));
            if (allNamesPresent)
            {
                var asNames = unassigned.Select(s => s.Name);
                if (asNames.Distinct().Count() == unassigned.Count)
                {
                    foreach (IfcSpace s in unassigned)
                    { 
                        res[s.Guid] = s.Name; 
                    }
                }
            }

            if (allNamesPresent && allLongNamesPresent)
            {
                Func<IfcSpace, string> combinedName = s =>
                    String.Format("{0} {1}", s.LongName, s.Name);
                var combined = unassigned.Select(combinedName);
                if (combined.Distinct().Count() == unassigned.Count)
                {
                    foreach (IfcSpace s in unassigned) {
                        res[s.Guid] = combinedName(s); 
                    }
                    return res;
                }
            }

            // TODO: check for case-sensitivity collisions here (ugh revit)
            foreach (IfcSpace s in unassigned) { res[s.Guid] = s.Guid; }
            return res;
        }

        public IdfGeneration(ViewModel vm, Action completionAction)
            : base(_ => vm.UpdateGlobalStatus(), () => vm.ReasonForDisabledIdfGeneration == null)
        {
            PrepareParameters = () => GetParameters(vm);
            PerformLongOperation = GenerateIdf;
            ProgressHandler = evt => vm.UpdateOutputDirectly(evt.Message);
            LongOperationComplete = _ => completionAction();
        }
    }
}
