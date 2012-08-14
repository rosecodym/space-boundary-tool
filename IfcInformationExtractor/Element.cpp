#include <cpp_edmi.h>

#include "CompositeConstruction.h"
#include "SingleMaterial.h"

#include "Element.h"

namespace IfcInformationExtractor {

namespace {

Construction ^ createSingleMaterial(const cppw::Instance & inst, String ^ elementGuid) {
	if (inst.is_kind_of("IfcMaterial")) {
		return gcnew SingleMaterial(gcnew String(((cppw::String)inst.get("Name")).data()));
	}
	else if (inst.is_kind_of("IfcMaterialLayer")) {
		cppw::Select mat = inst.get("Material");
		return gcnew SingleMaterial(mat.is_set() ?
			gcnew String(((cppw::String)((cppw::Instance)mat).get("Name")).data()) : String::Format("(unset material name for layer - element {0})", elementGuid));
	}
	else {
		return gcnew SingleMaterial(String::Format("(material name for unknown source - element {0})", elementGuid));
	}
}

Construction ^ createConstructionForLayerSet(const cppw::Instance & inst, String ^ elementGuid) {
	cppw::List mats = inst.get("MaterialLayers");
	if (mats.count() > 1) {
		cppw::Select name = inst.get("LayerSetName");
		return gcnew CompositeConstruction(name.is_set() ? 
			gcnew String(((cppw::String)name).data()) : String::Format("(unnamed composite construction - element {0})", elementGuid));
	}
	else {
		return createSingleMaterial((cppw::Instance)mats.get_(0), elementGuid);
	}
}

Construction ^ createConstruction(const cppw::Instance & inst, String ^ elementGuid) {
	if (inst.is_kind_of("IfcMaterial") || inst.is_kind_of("IfcMaterialLayer")) {
		return createSingleMaterial(inst, elementGuid);
	}
	else if (inst.is_kind_of("IfcMaterialLayerSet")) {
		return createConstructionForLayerSet(inst, elementGuid);
	}
	else if (inst.is_kind_of("IfcMaterialLayerSetUsage")) {
		return createConstructionForLayerSet((cppw::Instance)inst.get("ForLayerSet"), elementGuid);
	}
	else if (inst.is_kind_of("IfcMaterialList")) {
		return gcnew CompositeConstruction(String::Format("(construction for material list - element {0})", elementGuid));
	}
	else {
		return gcnew SingleMaterial(String::Format("(material for unknown material source - element {0})", elementGuid));
	}
}

Construction ^ createConstructionForWindow(const cppw::Instance & element, String ^ elementGuid) {
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
	return gcnew CompositeConstruction(String::Format("(undeterminable window construction - element {0})", elementGuid));
}

Construction ^ createConstructionForCommon(const cppw::Instance & element, String ^ elementGuid) {
	cppw::Set relAssociates = element.get("HasAssociations");
	for (relAssociates.move_first(); relAssociates.move_next(); ) {
		cppw::Instance rel = relAssociates.get_();
		if (rel.is_kind_of("IfcRelAssociatesMaterial")) {
			return createConstruction((cppw::Instance)rel.get("RelatingMaterial"), elementGuid);
		}
	}
	return gcnew SingleMaterial(String::Format("(material for missing material source - element {0})", elementGuid));
}

Construction ^ createConstructionFor(const cppw::Instance & buildingElement, String ^ elementGuid) {
	if (buildingElement.is_kind_of("IfcWindow")) {
		return createConstructionForWindow(buildingElement, elementGuid);
	}
	else {
		return createConstructionForCommon(buildingElement, elementGuid);
	}
}

} // namespace

Element::Element(const cppw::Instance & inst) : guid(gcnew String(((cppw::String)inst.get("GlobalId")).data()))
{ 
	construction = createConstructionFor(inst, guid);
}

} // namespace IfcInformationExtractor