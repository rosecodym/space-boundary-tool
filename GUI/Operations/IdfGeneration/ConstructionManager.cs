using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using LibraryEntry = GUI.Materials.LibraryEntries.Opaque;
using OutputConstruction = GUI.Materials.Output.Construction;
using OutputLayer = GUI.Materials.Output.MaterialLayer;

namespace GUI.Operations
{
    static partial class IdfGeneration
    {
        class ConstructionManager
        {
            private Func<int, LibraryEntry> sbtMaterialIDToLibraryMaterial;
            private HashSet<OutputLayer> allMaterials = new HashSet<OutputLayer>();
            private HashSet<OutputConstruction> allConstructions = new HashSet<OutputConstruction>();

            public ConstructionManager(Func<int, LibraryEntry> sbtMaterialIDToLibraryMaterial)
            {
                this.sbtMaterialIDToLibraryMaterial = sbtMaterialIDToLibraryMaterial;
            }

            public string ConstructionNameForMaterials(IList<Sbt.CoreTypes.MaterialLayer> materials)
            {
                List<OutputLayer> inNewIdf = new List<OutputLayer>();
                foreach (Sbt.CoreTypes.MaterialLayer layer in materials)
                {
                    LibraryEntry inLibrary = sbtMaterialIDToLibraryMaterial(layer.Id);
                    if (inLibrary == null) { return "UNMAPPED CONSTRUCTION"; }
                    inNewIdf.Add(RetrieveMaterial(inLibrary, layer.Thickness));
                }
                return RetrieveConstruction(inNewIdf).Name;
            }

            private OutputLayer RetrieveMaterial(LibraryEntry libraryMaterial, double thickness)
            {
                OutputLayer newLayer = new Materials.Output.MaterialLayerOpaque(libraryMaterial.Name, libraryMaterial.Properties, thickness);
                allMaterials.Add(newLayer);
                return newLayer;
            }

            private OutputConstruction RetrieveConstruction(IList<OutputLayer> materials)
            {
                OutputConstruction newC = new OutputConstruction(materials);
                allConstructions.Add(newC);
                return newC;
            }
        }
    }
}
