﻿using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;

using ConstructionManager = ConstructionManagement.OutputManager;
using IdfConstruction = ConstructionManagement.OutputConstruction;
using IdfMaterial = ConstructionManagement.OutputLayer;

using IfcConstruction = ConstructionManagement.ModelConstructions.ModelConstruction;
using IfcElement = IfcInformationExtractor.Element;
using IfcSpace = IfcInformationExtractor.Space;

using LibraryEntry = ConstructionManagement.MaterialLibrary.LibraryEntry;

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
                ConstructionManager cmanager = new ConstructionManager(msg => ReportProgress(msg + Environment.NewLine, ProgressEvent.ProgressEventType.Warning));
                IDictionary<string, string> zoneNamesByGuid = GatherZoneNamesByGuid(FindUsedSpaces(p.SbtBuilding.SpaceBoundaries, p.IfcBuilding.SpacesByGuid));
                Func<int, IfcConstruction> materialIdToModelConstruction = id =>
                {
                    if (id > p.SbtBuilding.Elements.Count) { return null; }
                    string elementGuid = p.SbtBuilding.Elements[id - 1].Guid;
                    IfcElement ifcElement;
                    if (!p.IfcBuilding.ElementsByGuid.TryGetValue(elementGuid, out ifcElement)) { return null; }
                    return ifcElement.AssociatedConstruction;
                };
                IDictionary<string, BuildingSurface> surfacesByGuid = new Dictionary<string, BuildingSurface>();
                foreach (Sbt.CoreTypes.SpaceBoundary sb in p.SbtBuilding.SpaceBoundaries.Where(sb => sb.IsVirtual || !sb.Element.IsFenestration))
                {
                    if (sb.Level == 2)
                    {
                        List<IfcConstruction> constructions = new List<IfcConstruction>();
                        List<double> thicknesses = new List<double>();
                        foreach (Sbt.CoreTypes.MaterialLayer layer in sb.MaterialLayers)
                        {
                            constructions.Add(p.MaterialIDToModelConstruction(layer.Id));
                            thicknesses.Add(layer.Thickness);
                        }
                        surfacesByGuid[sb.Guid] = new BuildingSurface(
                            sb,
                            cmanager.ConstructionNameForLayers(constructions, thicknesses),
                            zoneNamesByGuid[sb.BoundedSpace.Guid]);
                    }
                    else
                    {
                        surfacesByGuid[sb.Guid] = new BuildingSurface(
                            sb,
                            cmanager.ConstructionNameForSurface(p.MaterialIDToModelConstruction(sb.Element.MaterialId)),
                            zoneNamesByGuid[sb.BoundedSpace.Guid]);
                    }
                }
                List<FenestrationSurface> fenestrations = new List<FenestrationSurface>();
                foreach (Sbt.CoreTypes.SpaceBoundary sb in p.SbtBuilding.SpaceBoundaries)
                {
                    // all fenestration sbs *should* have a containing boundary, but this doesn't always happen
                    if (!sb.IsVirtual && sb.Element.IsFenestration && sb.ContainingBoundary != null)
                    {
                        List<IfcConstruction> constructions = new List<IfcConstruction>();
                        List<double> thicknesses = new List<double>();
                        foreach (Sbt.CoreTypes.MaterialLayer layer in sb.MaterialLayers)
                        {
                            constructions.Add(p.MaterialIDToModelConstruction(layer.Id));
                            thicknesses.Add(layer.Thickness);
                        }
                        fenestrations.Add(new FenestrationSurface(sb, surfacesByGuid[sb.ContainingBoundary.Guid], cmanager.ConstructionNameForLayers(constructions, thicknesses)));
                    }
                }
                IDictionary<string, Sbt.CoreTypes.Solid> elementGeometriesByGuid = new Dictionary<string, Sbt.CoreTypes.Solid>();
                foreach (Sbt.CoreTypes.ElementInfo element in p.SbtBuilding.Elements)
                {
                    elementGeometriesByGuid[element.Guid] = element.Geometry;
                }

                List<BuildingSurface> wallBoundaries = new List<BuildingSurface>(surfacesByGuid.Values.Where(surf => surf.HasElementType(Sbt.CoreTypes.ElementType.Wall)));
                var shadings =
                    p.IfcBuilding.ElementsByGuid.Values
                    .Where(element => element.IsShading)
                    .Select(element => new Shading(element.Guid, elementGeometriesByGuid[element.Guid], wallBoundaries));
                IdfCreator creator = IdfCreator.Build(p.EPVersion, idd, msg => ReportProgress(msg));
                creator.AddConstantContents();
                creator.AddLocation(p.LocationName, p.TimeZone, p.IfcBuilding.Latitude, p.IfcBuilding.Longitude, p.IfcBuilding.Elevation);
                creator.AddBuilding(p.NorthAxis, p.LoadsConvergenceTolerance, p.TemperatureConvergenceTolerance, p.SolarDistribution, p.BuildingTerrain);
                creator.AddTimestep(p.Timestep);
                creator.AddRunPeriod(p.StartMonth, p.StartDay, p.EndMonth, p.EndDay);
                foreach (KeyValuePair<string, string> zone in zoneNamesByGuid) { creator.AddZone(zone.Value, zone.Key); }
                foreach (BuildingSurface surf in surfacesByGuid.Values) { creator.AddBuildingSurface(surf); }
                foreach (FenestrationSurface fen in fenestrations) { creator.AddFenestration(fen); }
                foreach (Shading s in shadings) { creator.AddShading(s); }
                foreach (IdfConstruction c in cmanager.AllOutputConstructions) { creator.AddConstruction(c); }
                foreach (IdfMaterial m in cmanager.AllOutputLayers) { creator.AddMaterial(m); }
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