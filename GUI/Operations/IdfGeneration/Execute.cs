﻿using System;
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
            if (!vm.Busy)
            {
                try
                {
                    vm.Busy = true;
                    // TODO: check for pre-existing building
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
                        vm.Busy = false;
                    });

                    Parameters p = new Parameters();
                    p.OutputFilename = vm.OutputIdfFilePath;
                    p.SbtBuilding = vm.CurrentSbtBuilding;
                    p.IfcBuilding = vm.CurrentIfcBuilding;
                    p.IfcConstructionsByName = new Dictionary<string, IfcConstruction>();
                    foreach (IfcConstruction c in vm.IfcConstructions)
                    {
                        p.IfcConstructionsByName[c.Name] = c;
                    }
                    p.GetIdd = () => vm.Idds.GetIddFor((EnergyPlusVersion)vm.EnergyPlusVersionIndexToWrite, msg => worker.ReportProgress(0, msg + Environment.NewLine));
                    p.Notify = msg => worker.ReportProgress(0, msg);

                    worker.RunWorkerAsync(p);
                }
                catch (Exception)
                {
                    vm.Busy = false;
                }
            }
        }

        static private void DoIdfGenerationWork(object sender, DoWorkEventArgs e)
        {
            Parameters p = e.Argument as Parameters;
            if (p != null)
            {
                try
                {
                    p.Notify("Getting IDD.\n");
                    LibIdf.Idd.Idd idd = p.GetIdd();
                    p.Notify("Got IDD.\n");

                    ConstructionManager constructionManager = new ConstructionManager(id =>
                    {
                        if (id >= p.SbtBuilding.Elements.Count) { return null; }
                        string elementGuid = p.SbtBuilding.Elements[id - 1].Guid;
                        IfcInformationExtractor.Element ifcElement;
                        if (!p.IfcBuilding.ElementsByGuid.TryGetValue(elementGuid, out ifcElement)) { return null; }
                        IfcConstruction c;
                        return p.IfcConstructionsByName.TryGetValue(ifcElement.AssociatedConstruction.Name, out c) ? c.IdfMappingTarget : null;
                    });

                    foreach (Sbt.CoreTypes.SpaceBoundary sb in p.SbtBuilding.SpaceBoundaries)
                    {
                        if (sb.Level == 2) { constructionManager.ConstructionNameForLayerMaterials(sb.MaterialLayers); }
                        else { constructionManager.ConstructionNameForSurfaceMaterial(sb.Element.MaterialId); }
                    }

                    IdfCreator creator = IdfCreator.Build(p.EPVersion, idd, p.Notify);

                    creator.AddConstantContents();
                    foreach (KeyValuePair<string, string> zone in GatherZoneNamesByGuid(FindUsedSpaces(p.SbtBuilding.SpaceBoundaries, p.IfcBuilding.SpacesByGuid)))
                    {
                        creator.AddZone(zone.Value, zone.Key);
                    }
                    foreach (Materials.Output.Construction c in constructionManager.AllConstructions) { creator.AddConstruction(c); }
                    foreach (Materials.Output.MaterialLayer m in constructionManager.AllMaterials) { creator.AddMaterial(m); }

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
