#include <cstring>
#include <cpp_edmi.h>

#include "Element.h"

namespace IfcInformationExtractor {

namespace {

ModelConstruction ^ createSingleMaterial(const cppw::Instance & inst, String ^ elementGuid, ModelConstructionCollection ^ constructions) {
	if (inst.is_kind_of("IfcMaterial")) {
		String ^ name = gcnew String(((cppw::String)inst.get("Name")).data());
		if (String::IsNullOrWhiteSpace(name)) {
			return constructions->GetModelConstructionSingleOpaque("(unnamed material)");
		}
		return constructions->GetModelConstructionSingleOpaque(name);
	}
	else if (inst.is_kind_of("IfcMaterialLayer")) {
		cppw::Select mat = inst.get("Material");
		if (mat.is_set()) { return createSingleMaterial((cppw::Instance)mat, elementGuid, constructions); }
		else { return constructions->GetModelConstructionSingleOpaque("(material for unspecified material in layer"); }
	}
	else {
		return constructions->GetModelConstructionSingleOpaque("(material for unknown material representation");
	}
}

ModelConstruction ^ createConstructionForLayerSet(const cppw::Instance & inst, String ^ /*elementGuid*/, ModelConstructionCollection ^ constructions) {
	cppw::List mats = inst.get("MaterialLayers");
	List<String ^> ^ names = gcnew List<String ^>();
	List<double> ^ thicknesses = gcnew List<double>();
	for (mats.move_first(); mats.move_next(); ) {
		cppw::Instance layer = mats.get_();
		cppw::Select name = layer.get("Material");
		if (name.is_set()) { names->Add(gcnew String(((cppw::String)name).data())); }
		else { names->Add(gcnew String("(material unspecified material in layer)")); }
		thicknesses->Add(layer.get("LayerThickness"));
	}
	return constructions->GetModelConstructionComposite(names, thicknesses);
}

ModelConstruction ^ createConstruction(const cppw::Instance & inst, String ^ elementGuid, ModelConstructionCollection ^ constructions) {
	if (inst.is_kind_of("IfcMaterial") || inst.is_kind_of("IfcMaterialLayer")) {
		return createSingleMaterial(inst, elementGuid, constructions);
	}
	else if (inst.is_kind_of("IfcMaterialLayerSet")) {
		return createConstructionForLayerSet(inst, elementGuid, constructions);
	}
	else if (inst.is_kind_of("IfcMaterialLayerSetUsage")) {
		return createConstructionForLayerSet((cppw::Instance)inst.get("ForLayerSet"), elementGuid, constructions);
	}
	else if (inst.is_kind_of("IfcMaterialList")) {
		return constructions->GetModelConstructionSingleOpaque("(construction for material list)");
	}
	else {
		return constructions->GetModelConstructionSingleOpaque("(construction for unknown material representation)");
	}
}

ModelConstruction ^ createConstructionForWindow(const cppw::Instance & element, String ^ /*elementGuid*/, ModelConstructionCollection ^ constructions) {
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
								return constructions->GetModelConstructionWindow(gcnew String(((cppw::String)prop.get("NominalValue")).data()));
							}
						}
					}
				}
			}
		}
	}
	return constructions->GetModelConstructionWindow("(undefined window construction)");
}

ModelConstruction ^ createConstructionForCommon(const cppw::Instance & element, String ^ elementGuid, ModelConstructionCollection ^ constructions) {
	cppw::Set relAssociates = element.get("HasAssociations");
	for (relAssociates.move_first(); relAssociates.move_next(); ) {
		cppw::Instance rel = relAssociates.get_();
		if (rel.is_kind_of("IfcRelAssociatesMaterial")) {
			return createConstruction((cppw::Instance)rel.get("RelatingMaterial"), elementGuid, constructions);
		}
	}
	return constructions->GetModelConstructionSingleOpaque("(construction for missing material properties");
}

ModelConstruction ^ createConstructionFor(const cppw::Instance & buildingElement, String ^ elementGuid, ModelConstructionCollection ^ constructions) {
	if (buildingElement.is_kind_of("IfcWindow")) {
		return createConstructionForWindow(buildingElement, elementGuid, constructions);
	}
	else {
		return createConstructionForCommon(buildingElement, elementGuid, constructions);
	}
}

bool detectShading(const cppw::Instance & inst) {
	cppw::Select name = inst.get("Name");
	if (name.is_set() && strstr(((cppw::String)name).data(), "Shading")) {
		return true;
	}
	cppw::Set defined_by = inst.get("IsDefinedBy");
	for (defined_by.move_first(); defined_by.move_next(); ) {
		cppw::Instance d = defined_by.get_();
		if (d.is_instance_of("IfcRelDefinesByProperties")) {
			cppw::Instance pset = d.get("RelatingPropertyDefinition");
			if (pset.get("Name").is_set()) {
				cppw::String name = pset.get("Name");
				if (strstr(name.data(), "ElementShading")) {
					return true;
				}
			}
		}
	}
	return false;
}

} // namespace

Element::Element(const cppw::Instance & inst, ModelConstructionCollection ^ constructions) 
	: guid(gcnew String(((cppw::String)inst.get("GlobalId")).data())),
	construction(createConstructionFor(inst, guid, constructions)),
	isShading(detectShading(inst))
{ }

} // namespace IfcInformationExtractor