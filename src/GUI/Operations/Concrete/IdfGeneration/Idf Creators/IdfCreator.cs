using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using IdfToolbox.Idd;
using IdfToolbox.Idf;

using Construction = ConstructionManagement.OutputConstruction;
using Material = ConstructionManagement.OutputLayer;

namespace GUI.Operations
{
    partial class IdfGeneration
    {
        abstract class IdfCreator
        {
            protected readonly Idf idf;
            protected readonly Action<string> notify;

            protected IdfCreator(Idf idf, Action<string> notify)
            {
                this.idf = idf;
                this.notify = notify;
            }

            public void WriteToFile(string filename)
            {
                idf.Write(
                    filename, 
                    IdfToolbox.Idf.ValidationChecks.None,
                    CommentPosition.Below,
                    false);
            }

            public static IdfCreator Build(EnergyPlusVersion version, Idd idd, Action<string> notify)
            {
                Idf idf = new Idf(idd);
                idf.SuspendReferenceUpdates();
                if (version == EnergyPlusVersion.V710)
                {
                    return new IdfV710Creator(idf, notify);
                }
                else if (version == EnergyPlusVersion.V720)
                {
                    return new IdfV720Creator(idf, notify);
                }
                else if (version == EnergyPlusVersion.V720C)
                {
                    return new IdfV720CCreator(idf, notify);
                }
                else if (version == EnergyPlusVersion.V800)
                {
                    return new IdfV800Creator(idf, notify);
                }
                else
                {
                    throw new ArgumentException("Unsupported EnergyPlus version.\n");
                }
            }

            public abstract void AddBuilding(double northAxis, double loadsConvergence, double tempConvergence, SolarDistribution solarDistribution, BuildingTerrain terrain);
            public abstract void AddBuildingSurface(BuildingSurface surf);
            public abstract void AddConstantContents();
            public abstract void AddConstruction(Construction c);
            public abstract void AddFenestration(FenestrationSurface fen);
            public abstract void AddLocation(string name, double timeZone, double latitude, double longitude, double elevation);
            public abstract void AddMaterial(Material layer);
            public abstract void AddRunPeriod(int startMonth, int startDay, int endMonth, int endDay);
            public abstract void AddShading(Shading shading);
            public abstract void AddTimestep(int timestep);
            public abstract void AddZone(string name);
        }
    }
}
