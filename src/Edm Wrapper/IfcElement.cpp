#include "IfcElement.h"

using namespace System;
using namespace System::Collections::Generic;

namespace IfcInterface {

namespace {

String ^ aggregateList(const cppw::Instance & inst) {
	cppw::List mats = inst.get("Materials");
	List<String ^> ^ names = gcnew List<String ^>();
	for (mats.move_first(); mats.move_next(); ) {
		cppw::Instance mat = mats.get_();
		cppw::String name = mat.get("Name");
		names->Add(gcnew String(name.data()));
	}
	return "Material list: " + String::Join(", ", names);
}

String ^ materialName(const cppw::Instance & inst) {
	if (inst.is_kind_of("IfcMaterial")) {
		String ^ name = gcnew String(((cppw::String)inst.get("Name")).data());
		if (String::IsNullOrWhiteSpace(name)) {
			return gcnew String("(unnamed material)");
		}
		else { return name; }
	}
	else if (inst.is_kind_of("IfcMaterialLayer")) {
		cppw::Select mat = inst.get("Material");
		if (mat.is_set()) { return materialName((cppw::Instance)mat); }
		else { 
			return gcnew String("(material for unspecified material in layer"); 
		}
	}
	else {
		return gcnew String("(material for unknown material representation)");
	}
}

Tuple<String ^, List<String ^> ^, List<double> ^> ^ processLayerSet(
	const cppw::Instance & inst)
{
	cppw::List mats = inst.get("MaterialLayers");
	List<String ^> ^ names = gcnew List<String ^>();
	List<double> ^ thicknesses = gcnew List<double>();
	for (mats.move_first(); mats.move_next(); ) {
		cppw::Instance layer = mats.get_();
		cppw::Select mat = layer.get("Material");
		if (mat.is_set()) {
			cppw::String name = ((cppw::Instance)mat).get("Name");
			names->Add(gcnew String(name.data()));
		}
		else {
			names->Add(
				gcnew String("(material for unspecified material in layer)"));
		}
		thicknesses->Add(layer.get("LayerThickness"));
	}
	cppw::Select name = inst.get("LayerSetName");
	if (name.is_set()) {
		String ^ setName = gcnew String(((cppw::String)name).data());
		return Tuple::Create(setName, names, thicknesses);
	}
	else { 
		return Tuple::Create(safe_cast<String ^>(nullptr), names, thicknesses); 
	}
}

Tuple<String ^, List<String ^> ^, List<double> ^> ^ createConstruction(
	const cppw::Select & sel)
{
	if (!sel.is_set()) { return nullptr; }
	cppw::Instance inst(sel);
	List<String ^> ^ names;
	List<double> ^ thicknesses;
	if (inst.is_kind_of("IfcMaterial")) {
		names = gcnew List<String ^>();
		names->Add(materialName(inst));
	}
	else if (inst.is_kind_of("IfcMaterialLayer")) {
		names = gcnew List<String ^>();
		names->Add(materialName(inst));
		thicknesses = gcnew List<double>();
		thicknesses->Add(inst.get("LayerThickness"));
	}
	else if (inst.is_kind_of("IfcMaterialLayerSet")) {
		return processLayerSet(inst);
	}
	else if (inst.is_kind_of("IfcMaterialLayerSetUsage")) {
		return processLayerSet((cppw::Instance)inst.get("ForLayerSet"));
	}
	else if (inst.is_kind_of("IfcMaterialList")) { 
		names = gcnew List<String ^>();
		names->Add(aggregateList(inst));
	}
	else { return nullptr; }
	return Tuple::Create(safe_cast<String ^>(nullptr), names, thicknesses);
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

String ^ windowConstructionName(const cppw::Instance & element) {
	cppw::Set defined_by = element.get("IsDefinedBy");
	for (defined_by.move_first(); defined_by.move_next(); ) {
		cppw::Instance d = defined_by.get_();
		if (d.is_instance_of("IfcRelDefinesByProperties")) {
			cppw::Instance pset = d.get("RelatingPropertyDefinition");
			if (pset.get("Name").is_set()) {
				cppw::String name = pset.get("Name");
				if (name.size() >= 11 && 
					!strncmp(name.data(), "PSet_Window", 11)) 
				{
					cppw::Set props = pset.get("HasProperties");
					for (props.move_first(); props.move_next(); ) {
						cppw::Instance prop = props.get_();
						if (prop.is_instance_of("IfcPropertySingleValue") && 
							prop.get("NominalValue").is_set()) 
						{
							if (prop.get("Name") == "ConstructionName" || 
								prop.get("Name") == "Reference") 
							{
								cppw::String val = prop.get("NominalValue");
								return gcnew String(val.data());
							}
						}
					}
				}
			}
		}
	}
	return gcnew String("(Generic Window)");
}

}

IfcElement::IfcElement(const cppw::Instance & inst)
	: guid(gcnew String(((cppw::String)inst.get("GlobalId")).data())),
	  isShading(detectShading(inst)),
	  isWindow(inst.is_kind_of("IfcWindow"))
{
	if (isWindow) {
		layerNames = gcnew List<String ^>();
		layerNames->Add(windowConstructionName(inst));
	}
	else if (inst.is_kind_of("IfcDoor")) {
		layerNames = gcnew List<String ^>();
		layerNames->Add(gcnew String("(Generic Door)"));
	}
	else {
		cppw::Set relAssociates = inst.get("HasAssociations");
		for (relAssociates.move_first(); relAssociates.move_next(); ) {
			cppw::Instance rel = relAssociates.get_();
			if (rel.is_kind_of("IfcRelAssociatesMaterial")) {
				Tuple<String ^, List<String ^> ^, List<double> ^> ^ cdata;
				cdata = createConstruction(rel.get("RelatingMaterial"));
				if (cdata != nullptr) {
					compositeName = cdata->Item1;
					layerNames = cdata->Item2;
					layerThicknesses = cdata->Item3;
					return;
				}
			}
		}
	}
}

} // namespace IfcInterface