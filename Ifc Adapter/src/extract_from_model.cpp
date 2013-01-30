#include "precompiled.h"

#include "model_operations.h"

#include "add_element.h"
#include "internal_geometry.h"
#include "unit_scaler.h"

namespace {

template <typename T>
T ** create_list(size_t count) {
	T ** res = (T **)malloc(sizeof(T *) * count);
	for (size_t i = 0; i < count; ++i) {
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
	double ** composite_layer_dxs,
	double ** composite_layer_dys,
	double ** composite_layer_dzs,
	void (*msg_func)(char *), 
	void (*warn_func)(char *),
	const unit_scaler & s, 
	const std::function<bool(const char *)> & passes_filter, 
	number_collection<K> * c,
	std::vector<element_info *> * shadings)
{
	std::vector<element_info *> infos;
	std::vector<direction_3> composite_dirs;

	int next_element_id = 1;

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
				add_element(
					&infos, 
					&composite_dirs,
					type, 
					elem, 
					msg_func, 
					warn_func,
					s, 
					next_element_id++, 
					c);
			}
			else if (shadings != nullptr) {
				add_element(
					shadings, 
					nullptr, 
					type, 
					elem, 
					msg_func, 
					warn_func, 
					s, 
					-1, 
					c);
			}
		}
	}

	char buf[256];
	sprintf(buf, "Creating list for %u elements.\n", infos.size());
	msg_func(buf);
	*elements = create_list<element_info>(infos.size());
	*composite_layer_dxs = (double *)calloc(infos.size(), sizeof(double));
	*composite_layer_dys = (double *)calloc(infos.size(), sizeof(double));
	*composite_layer_dzs = (double *)calloc(infos.size(), sizeof(double));
	msg_func("Element list created.\n");

	for (size_t i = 0; i < infos.size(); ++i) {
		(*elements)[i] = infos[i];
		(*composite_layer_dxs)[i] = CGAL::to_double(composite_dirs[i].dx());
		(*composite_layer_dys)[i] = CGAL::to_double(composite_dirs[i].dy());
		(*composite_layer_dzs)[i] = CGAL::to_double(composite_dirs[i].dz());
	}

	msg_func("All elements added to list.\n");
	return infos.size();
}

size_t get_spaces(
	cppw::Open_model & model, 
	space_info *** spaces, 
	void (*msg_func)(char *), 
	void (*warn_func)(char *),
	const unit_scaler & s, 
	const std::function<bool(const char *)> & space_filter, 
	number_collection<K> * c)
{
	using internal_geometry::get_local_geometry;
	using internal_geometry::get_globalizer;
	typedef boost::format fmt;
		
	auto ss = model.get_set_of("IfcSpace");
	std::vector<space_info> space_vector;

	for (ss.move_first(); ss.move_next(); ) {
		std::string id(((cppw::String)ss.get().get("GlobalId")).data());
		try {
			if (space_filter(id.c_str())) {
				fmt m("Extracting space %s...");
				msg_func(const_cast<char *>((m % id).str().c_str()));
				auto geometry = get_local_geometry(ss.get(), s, c);
				auto globalizer = get_globalizer(ss.get(), s, c);
				geometry->transform(globalizer);
				space_info this_space;
				strncpy(this_space.id, id.c_str(), SPACE_ID_MAX_LEN);
				this_space.geometry = geometry->to_interface_solid();
				space_vector.push_back(this_space);
				msg_func("done.\n");
			}
		}
		catch (internal_geometry::bad_rep_exception & ex) {
			fmt m("Space %s could not be loaded (%s). It will be skipped.\n");
			warn_func(const_cast<char *>((m % id % ex.what()).str().c_str()));
		}
	}

	*spaces = create_list<space_info>(space_vector.size());
	for (size_t i = 0; i < space_vector.size(); ++i) {
		*(*spaces)[i] = space_vector[i];
	}
	return space_vector.size();
}

} // namespace

ifcadapter_return_t extract_from_model(
	cppw::Open_model & model, 
	size_t * element_count, 
	element_info *** elements, 
	double ** composite_layer_dxs,
	double ** composite_layer_dys,
	double ** composite_layer_dzs,
	size_t * space_count,
	space_info *** spaces,
	void (*notify)(char *),
	void (*warn)(char *),
	const std::function<bool(const char *)> & element_filter,
	const std::function<bool(const char *)> & space_filter,
	number_collection<K> * c,
	std::vector<element_info *> * shadings)
{
	auto scaler = unit_scaler::identity_scaler;
	char buf[256];
	*element_count = get_elements(
		model, 
		elements, 
		composite_layer_dxs,
		composite_layer_dys,
		composite_layer_dzs,
		notify, 
		warn,
		scaler, 
		element_filter, 
		c, 
		shadings);
	sprintf(buf, "Got %u building elements.\n", *element_count);
	notify(buf);
	*space_count = 
		get_spaces(model, spaces, notify, warn, scaler, space_filter, c);
	sprintf(buf, "Got %u building spaces.\n", *space_count);
	notify(buf);
	return IFCADAPT_OK;
}