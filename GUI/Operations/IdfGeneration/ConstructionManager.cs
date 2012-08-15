using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace GUI.Operations
{
    static partial class IdfGeneration
    {
        class ConstructionManager
        {
            private Func<int, Constructions.MaterialLayer> sbtMaterialIDToLibraryMaterial;

            public ConstructionManager(Func<int, Constructions.MaterialLayer> sbtMaterialIDToLibraryMaterial)
            {
                this.sbtMaterialIDToLibraryMaterial = sbtMaterialIDToLibraryMaterial;
            }

            public string ConstructionNameForMaterials(IList<Sbt.CoreTypes.MaterialLayer> materials)
            {
                List<Constructions.MaterialLayer> inNewIdf = new List<Constructions.MaterialLayer>();
                foreach (Sbt.CoreTypes.MaterialLayer layer in materials)
                {
                    Constructions.MaterialLayer inLibrary = sbtMaterialIDToLibraryMaterial(layer.Id);
                    if (inLibrary == null) { return "UNMAPPED CONSTRUCTION"; }
                    inNewIdf.Add(RetrieveMaterial(inLibrary, layer.Thickness));
                }
                return RetrieveConstruction(inNewIdf).Name;
            }

            private Constructions.MaterialLayer RetrieveMaterial(Constructions.MaterialLayer libraryMaterial, double thickness)
            {
                throw new NotImplementedException();
            }
            private Constructions.Construction RetrieveConstruction(IList<Constructions.MaterialLayer> materials)
            {
                throw new NotImplementedException();
            }
        }
    }
}
