#include <cpp_edmi.h>

#include "ConstructionFactory.h"

#include "Element.h"

namespace IfcInformationExtractor {

namespace {

Construction ^ createSingleMaterial(const cppw::Instance & inst, String ^ elementGuid, ConstructionFactory ^ constructionFactory) {
	if (inst.is_kind_of("IfcMaterial")) {
		String ^ name = gcnew String(((cppw::String)inst.get("Name")).data());
		if (String::IsNullOrWhiteSpace(name)) {
			return constructionFactory->GetUnnamedMaterial(elementGuid);
		}
		return constructionFactory->GetSingleMaterial(name);
	}
	else if (inst.is_kind_of("IfcMaterialLayer")) {
		cppw::Select mat = inst.get("Material");
		if (mat.is_set()) { return createSingleMaterial((cppw::Instance)mat, elementGuid, constructionFactory); }
		else { return constructionFactory->GetUnnamedMaterial(elementGuid); }
	}
	else {
		return constructionFactory->GetUnknownMaterial(elementGuid);
	}
}

Construction ^ createConstructionForLayerSet(const cppw::Instance & inst, String ^ elementGuid, ConstructionFactory ^ constructionFactory) {
	cppw::List mats = inst.get("MaterialLayers");
	if (mats.count() > 1) {
		cppw::Select name = inst.get("LayerSetName");
		return name.is_set() ?
			constructionFactory->GetComposite(gcnew String(((cppw::String)name).data())) : constructionFactory->GetUnnamedComposite(elementGuid);
	}
	else {
		return createSingleMaterial((cppw::Instance)mats.get_(0), elementGuid, constructionFactory);
	}
}

Construction ^ createConstruction(const cppw::Instance & inst, String ^ elementGuid, ConstructionFactory ^ constructionFactory) {
	if (inst.is_kind_of("IfcMaterial") || inst.is_kind_of("IfcMaterialLayer")) {
		return createSingleMaterial(inst, elementGuid, constructionFactory);
	}
	else if (inst.is_kind_of("IfcMaterialLayerSet")) {
		return createConstructionForLayerSet(inst, elementGuid, constructionFactory);
	}
	else if (inst.is_kind_of("IfcMaterialLayerSetUsage")) {
		return createConstructionForLayerSet((cppw::Instance)inst.get("ForLayerSet"), elementGuid, constructionFactory);
	}
	else if (inst.is_kind_of("IfcMaterialList")) {
		return constructionFactory->GetMaterialList(elementGuid);
	}
	else {
		return constructionFactory->GetUnknownComposite(elementGuid);
	}
}

Construction ^ createConstructionForWindow(const cppw::Instance & element, String ^ elementGuid, ConstructionFactory ^ constructionFactory) {
	cppw::Set defined_by = element.get("IsDefinedBy");
	for (defined_by.move_first(); defined_by.move_next(); ) {
		cppw::Instance d = defined_by.get_();
		if (d.is_instance_of("IfcRelDefinesByProperties")) {
			cppw::Instance pset = d.get("RelatingPropertyDefinition");
			if (pset.get("Name").is_set()) {
				cppw::String name = pset.get("Name");
				if (name.size() >= 11 && !strncmp(name.data(), "PSet_Window", 11)) {
					cppw::Set props = pset.get("HasProperties");
					for (props.move_first(); props.move_next(); ) {
						cppw::Instance prop = props.get_();
						if (prop.is_instance_of("IfcPropertySingleValue") && prop.get("NominalValue").is_set()) {
							if (prop.get("Name") == "ConstructionName" || prop.get("Name") == "Reference") {
								return gcnew CompositeConstruction(gcnew String(((cppw::String)prop.get("NominalValue")).data()));
							}
						}
					}
				}
			}
		}
	}
	return constructionFactory->GetMissingWindowMaterial(elementGuid);
}

Construction ^ createConstructionForCommon(const cppw::Instance & element, String ^ elementGuid, ConstructionFactory ^ constructionFactory) {
	cppw::Set relAssociates = element.get("HasAssociations");
	for (relAssociates.move_first(); relAssociates.move_next(); ) {
		cppw::Instance rel = relAssociates.get_();
		if (rel.is_kind_of("IfcRelAssociatesMaterial")) {
			return createConstruction((cppw::Instance)rel.get("RelatingMaterial"), elementGuid, constructionFactory);
		}
	}
	return constructionFactory->GetMissingMaterial(elementGuid);
}

Construction ^ createConstructionFor(const cppw::Instance & buildingElement, String ^ elementGuid, ConstructionFactory ^ constructionFactory) {
	if (buildingElement.is_kind_of("IfcWindow")) {
		return createConstructionForWindow(buildingElement, elementGuid, constructionFactory);
	}
	else {
		return createConstructionForCommon(buildingElement, elementGuid, constructionFactory);
	}
}

} // namespace

Element::Element(const cppw::Instance & inst, ConstructionFactory ^ constructionFactory) 
	: guid(gcnew String(((cppw::String)inst.get("GlobalId")).data())),
	construction(createConstructionFor(inst, guid, constructionFactory))
{ }

} // namespace IfcInformationExtractor