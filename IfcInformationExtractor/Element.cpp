#include <cpp_edmi.h>

#include "CompositeConstruction.h"
#include "SingleMaterial.h"

#include "Element.h"

namespace IfcInformationExtractor {

namespace {

Construction ^ createSingleMaterial(const cppw::Instance & inst) {
	if (inst.is_kind_of("IfcMaterial")) {
		return gcnew SingleMaterial(gcnew String(((cppw::String)inst.get("Name")).data()));
	}
	else if (inst.is_kind_of("IfcMaterialLayer")) {
		cppw::Select mat = inst.get("Material");
		return gcnew SingleMaterial (gcnew String(mat.is_set() ? 
			((cppw::String)((cppw::Instance)mat).get("Name")).data() : "(unset material name for layer)"));
	}
	else {
		return gcnew SingleMaterial(gcnew String("(material name for unknown source)"));
	}
}

Construction ^ createConstructionForLayerSet(const cppw::Instance & inst) {
	cppw::List mats = inst.get("MaterialLayers");
	if (mats.count() > 1) {
		cppw::Select name = inst.get("LayerSetName");
		return gcnew CompositeConstruction(gcnew String(name.is_set() ? 
			((cppw::String)name).data() : "(unnamed composite construction)"));
	}
	else {
		return createSingleMaterial((cppw::Instance)mats.get_(0));
	}
}

Construction ^ createConstruction(const cppw::Instance & inst) {
	if (inst.is_kind_of("IfcMaterial") || inst.is_kind_of("IfcMaterialLayer")) {
		return createSingleMaterial(inst);
	}
	else if (inst.is_kind_of("IfcMaterialLayerSet")) {
		return createConstructionForLayerSet(inst);
	}
	else if (inst.is_kind_of("IfcMaterialLayerSetUsage")) {
		return createConstructionForLayerSet((cppw::Instance)inst.get("ForLayerSet"));
	}
	else if (inst.is_kind_of("IfcMaterialList")) {
		return gcnew CompositeConstruction("(construction for material list)");
	}
	else {
		return gcnew SingleMaterial("(material for unknown material source)");
	}
}

Construction ^ createConstructionForCommon(const cppw::Instance & element) {
	cppw::Set relAssociates = element.get("HasAssociations");
	for (relAssociates.move_first(); relAssociates.move_next(); ) {
		cppw::Instance rel = relAssociates.get_();
		if (rel.is_kind_of("IfcRelAssociatesMaterial")) {
			return createConstruction((cppw::Instance)rel.get("RelatingMaterial"));
		}
	}
	return gcnew SingleMaterial("(material for missing material source)");
}

Construction ^ createConstructionFor(const cppw::Instance & buildingElement) {
	return createConstructionForCommon(buildingElement);
}

} // namespace

Element::Element(const cppw::Instance & inst)
	: guid(gcnew String(((cppw::String)inst.get("GlobalId")).data())),
	construction(createConstructionFor(inst))
{ }

} // namespace IfcInformationExtractor