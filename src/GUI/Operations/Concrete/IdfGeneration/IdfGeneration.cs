using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;

using IfcInterface;

using Solid = Sbt.CoreTypes.Solid;
using SbtLayer = Sbt.CoreTypes.MaterialLayer;
using SbtElement = Sbt.CoreTypes.ElementInfo;
using SbtElementType = Sbt.CoreTypes.ElementType;

using ConstructionManager = ConstructionManagement.OutputManager;
using IdfConstruction = ConstructionManagement.OutputConstruction;
using IdfMaterial = ConstructionManagement.OutputLayer;

using IfcConstruction = ConstructionManagement.ModelConstructions.ModelConstruction;

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
            p.EPVersion = (EnergyPlusVersion)vm.EnergyPlusVersionIndexToWrite;
            p.OutputFilename = vm.OutputIdfFilePath;
            p.LocationName = vm.BuildingLocation;
            p.Tolerance = vm.Tolerance;
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
                IdfToolbox.Idd.Idd idd = p.GetIdd();
                ReportProgress("Got IDD.\n");
                ConstructionManager cmanager =
                    new ConstructionManager(
                        msg =>
                            ReportProgress(
                                msg + Environment.NewLine,
                                ProgressEvent.ProgressEventType.Warning));
                var zoneAssignment = new ZoneAssignment(
                    p.SbtBuilding.SpaceBoundaries,
                    p.IfcBuilding.SpacesByGuid.Values);
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
                            zoneAssignment[sb.BoundedSpace.Guid] !=
                            zoneAssignment[sb.Opposite.BoundedSpace.Guid];
                    }
                });
                var areaCorrections = p.SbtBuilding.CorrectedAreas;
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
                        float? trueArea = null;
                        float corr;
                        if (areaCorrections.TryGetValue(sb.Guid, out corr))
                        {
                            trueArea = corr;
                        }
                        surfacesByGuid[sb.Guid] = new BuildingSurface(
                            sb,
                            cmanager.ConstructionForLayers(
                                sb.Normal,
                                constructions,
                                dirs,
                                thicknesses),
                            zoneAssignment[sb.BoundedSpace.Guid],
                            trueArea);
                    }
                    else
                    {
                        int mID = sb.Element.MaterialId;
                        var modelC = p.MaterialIDToModelConstruction(mID);
                        var modelCNorm = p.MaterialIDToCompositeDir(mID);
                        var c = cmanager.ConstructionForSurface(
                            sb.Normal,
                            modelCNorm,
                            modelC);
                        var zoneName = zoneAssignment[sb.BoundedSpace.Guid];
                        var surf = new BuildingSurface(sb, c, zoneName);
                        surfacesByGuid[sb.Guid] = surf;
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
                        cmanager.ConstructionForLayers(
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
                foreach (IfcElement e in p.IfcBuilding.Elements)
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
                var surfs = surfacesByGuid
                    .Values
                    .Where(surf => !surf.IsAdiabaticVirtual);
                var surfConstrs = surfs.Select(s => s.Construction);
                var fenConstrs = fenestrations.Select(f => f.Construction);
                cmanager.PruneOutput(surfConstrs.Concat(fenConstrs));
                IdfCreator creator =
                    IdfCreator.Build(
                        p.EPVersion,
                        idd,
                        msg => ReportProgress(msg));
                cmanager.IdentifyConstructionVariants();
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
                foreach (string zoneName in zoneAssignment.AllZoneNames())
                {
                    creator.AddZone(zoneName);
                }
                foreach (BuildingSurface surf in surfs)
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
