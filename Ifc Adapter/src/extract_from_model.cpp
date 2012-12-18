#include "precompiled.h"

#include "add_element.h"
#include "internal_geometry.h"
#include "unit_scaler.h"

namespace {

template <typename T>
T ** create_list(size_t size) {
	T ** res = (T **)malloc(sizeof(T *) * size);
	for (size_t i = 0; i < size; ++i) {
		res[i] = (T *)calloc(1, sizeof(T));
	}
	return res;
}

bool is_shading(const cppw::Instance & inst) {
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

size_t get_elements(
	cppw::Open_model & model, 
	element_info *** elements, 
	void (*msg_func)(char *), 
	const unit_scaler & s, 
	const std::function<bool(const char *)> & passes_filter, 
	number_collection<K> * c,
	std::vector<element_info *> * shadings)
{
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
		std::string guid(((cppw::String)elem.get("GlobalId")).data());
		if (type != UNKNOWN && passes_filter(guid.c_str())) {
			if (!is_shading(elem)) {
				add_element(&infos, type, elem, msg_func, s, next_material_id++, c);
			}
			else if (shadings != nullptr) {
				add_element(shadings, type, elem, msg_func, s, -1, c);
			}
		}
	}

	char buf[256];
	sprintf(buf, "Creating list for %u elements.\n", infos.size());
	msg_func(buf);
	*elements = create_list<element_info>(infos.size());
	msg_func("Element list created.\n");

	for (size_t i = 0; i < infos.size(); ++i) {
		(*elements)[i] = infos[i];
	}

	msg_func("All elements added to list.\n");
	return infos.size();
}

size_t get_spaces(cppw::Open_model & model, space_info *** spaces, void (*msg_func)(char *), const unit_scaler & s, const std::function<bool(const char *)> & space_filter, number_collection<K> * c) {
		
	auto ss = model.get_set_of("IfcSpace");
	size_t count = (size_t)ss.count();

	*spaces = create_list<space_info>(count);
		
	for (size_t i = 0; i < count; ++i) {
		try {
			strncpy((*spaces)[i]->id, ((cppw::String)ss.get(i).get("GlobalId")).data(), SPACE_ID_MAX_LEN);
			if (space_filter((*spaces)[i]->id)) {
				char buf[256];
				sprintf(buf, "Extracting space %s...", (*spaces)[i]->id);
				msg_func(buf);
				auto geometry = internal_geometry::get_local_geometry(
					ss.get(i),
					s,
					c);
				auto globalizer = internal_geometry::get_globalizer(
					ss.get(i),
					s,
					c);
				geometry->transform(globalizer);
				(*spaces)[i]->geometry = geometry->to_interface_solid();
				msg_func("done.\n");
			}
		}
		catch (internal_geometry::bad_rep_exception & ex) {
			char buf[256];
			sprintf(
				buf, 
				"Warning: could not load this space (%s). It will be skipped."
				"\n",
				ex.what());
			msg_func(buf);
		}
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
	void (*notify)(char *),
	const std::function<bool(const char *)> & element_filter,
	const std::function<bool(const char *)> & space_filter,
	number_collection<K> * c,
	std::vector<element_info *> * shadings)
{
	auto scaler = unit_scaler::identity_scaler;
	char buf[256];
	*element_count = get_elements(model, elements, notify, scaler, element_filter, c, shadings);
	sprintf(buf, "Got %u building elements.\n", *element_count);
	notify(buf);
	*space_count = get_spaces(model, spaces, notify, scaler, space_filter, c);
	sprintf(buf, "Got %u building spaces.\n", *space_count);
	notify(buf);
	return IFCADAPT_OK;
}