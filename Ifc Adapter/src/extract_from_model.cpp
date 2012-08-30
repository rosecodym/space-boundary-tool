#include "precompiled.h"

#include "add_element.h"
#include "ifc-to-solid.h"

class unit_scaler;

namespace {

size_t get_elements(cppw::Open_model & model, element_info *** elements, void (*msg_func)(char *), const unit_scaler & s, const std::function<bool(const char *)> & passes_filter, number_collection * c) {
	std::vector<element_info *> infos;

	int next_material_id = 1;

	auto building_elements = model.get_set_of("IfcBuildingElement", cppw::include_subtypes);
	for (building_elements.move_first(); building_elements.move_next(); ) {
		auto elem = building_elements.get();
		element_type type =
			elem.is_kind_of("IfcWall") ? WALL :
			elem.is_kind_of("IfcSlab") ? SLAB :
			elem.is_kind_of("IfcColumn") ? COLUMN :
			elem.is_kind_of("IfcBeam") ? BEAM :
			elem.is_kind_of("IfcDoor") ? DOOR :
			elem.is_kind_of("IfcWindow") ? WINDOW : 
			(elem.is_kind_of("IfcCovering") && elem.get("PredefinedType").is_set() && (cppw::String)elem.get("PredefinedType") == "CEILING") ? SLAB : UNKNOWN;
		if (type != UNKNOWN && passes_filter(((cppw::String)elem.get("GlobalId")).data())) {
			add_element(&infos, type, elem, msg_func, s, &next_material_id, c);
		}
	}

	char buf[256];
	sprintf(buf, "Creating list for %u elements.\n", infos.size());
	msg_func(buf);
	*elements = create_element_list(infos.size());
	msg_func("Element list created.\n");

	for (size_t i = 0; i < infos.size(); ++i) {
		(*elements)[i] = infos[i];
	}

	msg_func("All elements added to list.\n");

	return infos.size();
}

size_t get_spaces(cppw::Open_model & model, space_info *** spaces, const unit_scaler & s, number_collection * c) {
		
	auto ss = model.get_set_of("IfcSpace");
	size_t count = (size_t)ss.count();

	*spaces = create_space_list(count);
		
	for (size_t i = 0; i < count; ++i) {
		strncpy((*spaces)[i]->id, ((cppw::String)ss.get(i).get("GlobalId")).data(), SPACE_ID_MAX_LEN);
		exact_solid sld;
		ifc_to_solid(&sld, (cppw::Instance)ss.get(i), s, c);
		sld.populate_inexact_version(&(*spaces)[i]->geometry);
	}
	return count;
}

} // namespace

ifcadapter_return_t extract_from_model(
	cppw::Open_model & model, 
	size_t * element_count, 
	element_info *** elements,
	size_t * space_count,
	space_info *** spaces,
	void (*msg_func)(char *),
	const unit_scaler & s,
	const std::function<bool(const char *)> & element_filter,
	number_collection * c)
{
	char buf[256];
	*element_count = get_elements(model, elements, msg_func, s, element_filter, c);
	sprintf(buf, "Got %u building elements.\n", *element_count);
	msg_func(buf);
	*space_count = get_spaces(model, spaces, s, c);
	sprintf(buf, "Got %u building spaces.\n", *space_count);
	msg_func(buf);
	return IFCADAPT_OK;
}