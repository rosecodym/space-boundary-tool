#pragma once

#include "CompositeConstruction.h"
#include "Construction.h"
#include "BadMaterial.h"
#include "SingleMaterial.h"

using namespace System;

namespace IfcInformationExtractor {

public ref class ConstructionFactory {
private:
	BadMaterial ^ missingMaterial;
	BadMaterial ^ missingWindowMaterial;
	BadMaterial ^ materialList;
	BadMaterial ^ unknownCompositeSource;
	BadMaterial ^ unknownMaterialSource;
	BadMaterial ^ unnamedComposite;
	BadMaterial ^ unnamedMaterial;

public:
	ConstructionFactory()
		: missingMaterial(gcnew BadMaterial("(missing material)", "A construction representing a missing material in the IFC file.", false)),
		missingWindowMaterial(gcnew BadMaterial("(missing window material)", "A construction representing a missing window material in the IFC file.", true)),
		materialList(gcnew BadMaterial("(IFC material list)", "A construction representing an IFC material list, which are not supported.", false)),
		unknownCompositeSource(gcnew BadMaterial("(unsupported IFC construction type)", "A construction representing an IFC construction of an unknown IFC type.", false)),
		unknownMaterialSource(gcnew BadMaterial("(unsupported IFC material type)", "A construction representing an IFC material of an unknown IFC type.", false)),
		unnamedComposite(gcnew BadMaterial("(unnamed composite)", "A construction representing a composite construction with no name, or an empty name.", false)),
		unnamedMaterial(gcnew BadMaterial("(unnamed material)", "A construction representing a material with no name, or an empty name.", false))
	{ }

	Construction ^ GetSingleMaterial(String ^ name, bool forWindow) { return gcnew SingleMaterial(name, forWindow); }
	Construction ^ GetComposite(String ^ name, bool forWindow) { return gcnew CompositeConstruction(name, forWindow); }
	Construction ^ GetMissingMaterial(String ^ source) { missingMaterial->AddSource(source); return missingMaterial; }
	Construction ^ GetMissingWindowMaterial(String ^ source) { missingWindowMaterial->AddSource(source); return missingWindowMaterial; }
	Construction ^ GetMaterialList(String ^ source) { materialList->AddSource(source); return materialList; }
	Construction ^ GetUnknownComposite(String ^ source) { unknownCompositeSource->AddSource(source); return unknownCompositeSource; }
	Construction ^ GetUnknownMaterial(String ^ source) { unknownMaterialSource->AddSource(source); return unknownMaterialSource; }
	Construction ^ GetUnnamedComposite(String ^ source) { unnamedComposite->AddSource(source); return unnamedComposite; }
	Construction ^ GetUnnamedMaterial(String ^ source) { unnamedMaterial->AddSource(source); return unnamedMaterial; }
};

} // namespace IfcInformationExtractor