#include "precompiled.h"

#include "../../Edm Wrapper/edm_wrapper_native_interface.h"

#include "model_operations.h"

#include "internal_geometry.h"
#include "unit_scaler.h"

using namespace ifc_interface;

namespace {

using boost::optional;

template <typename T>
T ** create_list(size_t count) {
	T ** res = (T **)malloc(sizeof(T *) * count);
	for (size_t i = 0; i < count; ++i) {
		res[i] = (T *)calloc(1, sizeof(T));
	}
	return res;
}

bool is_shading(const ifc_object & obj) {
	std::string name;
	string_field(obj, "Name", &name);
	if (strstr(name.c_str(), "Shading")) { return true; }

	std::vector<ifc_object *> defined_by;
	collection_field(obj, "IsDefinedBy", &defined_by);
	for (auto d = defined_by.begin(); d != defined_by.end(); ++d) {
		if (is_instance_of(**d, "IfcRelDefinesByProperties")) {
			ifc_object * pset;
			object_field(**d, "RelatingPropertyDefinition", &pset);
			std::string name;
			string_field(*pset, "Name", &name);
			if (strstr(name.c_str(), "ElementShading")) {
				return true;
			}
		}
	}
	return false;
}

ifc_object * get_related_opening(const ifc_object & fen) {
	std::vector<ifc_object *> fills_voids;
	collection_field(fen, "FillsVoids", &fills_voids);
	ifc_object * res = nullptr;
	if (fills_voids.size() == 1) {
		object_field(*fills_voids.front(), "RelatingOpeningElement", &res);
	}
	return res;
}

boost::optional<direction_3> get_composite_dir(
	const ifc_object & obj,
	const direction_3 & axis1,
	const direction_3 & axis2,
	const direction_3 & axis3) 
{
	if (!is_kind_of(obj, "IfcWindow")) {
		std::vector<ifc_object *> rel_assoc;
		collection_field(obj, "HasAssociations", &rel_assoc);
		for (auto o = rel_assoc.begin(); o != rel_assoc.end(); ++o) {
			if (is_kind_of(**o, "IfcRelAssociatesMaterial")) {
				ifc_object * mat;
				object_field(**o, "RelatingMaterial", &mat);
				if (is_kind_of(*mat, "IfcMaterialLayerSetUsage")) {
					std::string dir, sense;
					string_field(*mat, "LayerSetDirection", &dir);
					string_field(*mat, "DirectionSense", &sense);
					direction_3 res =
						dir == "AXIS1" ? axis1 :
						dir == "AXIS2" ? axis2 :
										 axis3;
					if (sense == "NEGATIVE") { res = -res; }
					return res;
				}
			}
		}
	}
	return optional<direction_3>();
}

size_t get_elements(
	model * m,
	element_info *** elements, 
	double ** composite_layer_dxs,
	double ** composite_layer_dys,
	double ** composite_layer_dzs,
	void (*msg_func)(char *), 
	void (*warn_func)(char *),
	const unit_scaler & s, 
	const std::function<bool(const char *)> & passes_filter, 
	number_collection<K> * c,
	std::vector<element_info *> * shadings,
	std::vector<approximated_curve> * approximated_curves)
{
	std::vector<element_info *> infos;
	std::vector<direction_3> composite_dirs;
	*approximated_curves = std::vector<approximated_curve>();

	char buf[256];

	auto is_ceiling = [](const ifc_object & o) -> bool {
		if (!is_kind_of(o, "IfcCovering")) { return false; }
		std::string type;
		string_field(o, "PredefinedType", &type);
		return type == "CEILING";
	};

	auto bldg_elems = m->building_elements();
	for (auto e = bldg_elems.begin(); e != bldg_elems.end(); ++e) {
		element_type type =
			is_kind_of(**e, "IfcWall") ? WALL :
			is_kind_of(**e, "IfcSlab") ? SLAB :
			is_kind_of(**e, "IfcColumn") ? COLUMN :
			is_kind_of(**e, "IfcBeam") ? BEAM :
			is_kind_of(**e, "IfcDoor") ? DOOR :
			is_kind_of(**e, "IfcWindow") ? WINDOW : 
			is_ceiling(**e) ? SLAB : UNKNOWN;
		std::string guid;
		string_field(**e, "GlobalId", &guid);
		if (type != UNKNOWN && passes_filter(guid.c_str())) {
			sprintf(buf, "Extracting element %s...", guid.c_str());
			msg_func(buf);
			const ifc_object * effective_object = *e;
			if (type == WINDOW || type == DOOR) {
				effective_object = get_related_opening(**e);
				if (!effective_object) {
					sprintf(
						buf,
						"Door or window %s has no related opening. It will be "
						"skipped.\n",
						guid.c_str());
					warn_func(buf);
					continue;
				}
			}
			bool element_is_shading = is_shading(**e);
			std::unique_ptr<internal_geometry::solid> internal_geom;
			solid interface_geom;
			transformation_3 globalizer;
			direction_3 composite_dir(0, 0, 0);
			try {
				internal_geom = internal_geometry::get_local_geometry(
					*effective_object,
					s,
					c);
				globalizer = internal_geometry::get_globalizer(
					*effective_object,
					s,
					c);
				internal_geom->transform(globalizer);
				interface_geom = internal_geom->to_interface_solid();
				if (!element_is_shading && internal_geom->axis1()) {
					auto cdir_maybe = get_composite_dir(
						**e,
						*internal_geom->axis1(),
						*internal_geom->axis2(),
						*internal_geom->axis3());
					if (cdir_maybe) { composite_dir = *cdir_maybe; }
				}
				boost::copy(
					internal_geom->approximations(), 
					std::back_inserter(*approximated_curves));
			}
			catch (internal_geometry::bad_rep_exception & ex) {
				sprintf(
					buf, 
					"Element %s could not be loaded (%s). It will be "
					"skipped.\n",
					guid.c_str(),
					ex.what());
				warn_func(buf);
				continue;
			}
			element_info * info = (element_info *)malloc(sizeof(element_info));
			strncpy(info->name, guid.c_str(), ELEMENT_NAME_MAX_LEN);
			info->type = type;
			info->geometry = interface_geom;
			// info.id is poorly named. It represents the *material* id of the
			// element, and will be used by construction layers to identify this
			// element as the source of the layer material. The lookup is 
			// performed by treating the id as the index of the element in the
			// elements array; as such, the id must match the object's index!
			// Additionally, it shouldn't be needed for shading objects, since
			// they don't participate in construction assemblies. So we'll 
			// assign -1 to indicate "unset" first:
			info->id = -1;
			if (element_is_shading) { shadings->push_back(info); }
			else {
				// Then, if the element is a construction-participating element
				// (instead of a shading), we'll synchronize the indices:
				infos.push_back(info);
				infos.back()->id = infos.size();
				// The id is technically offset from the index by 1 for
				// historical reasons.
				composite_dirs.push_back(composite_dir);
			}
			msg_func("done.\n");
		}
	}

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
	model * m, 
	space_info *** spaces, 
	void (*msg_func)(char *), 
	void (*warn_func)(char *),
	const unit_scaler & s, 
	const std::function<bool(const char *)> & space_filter, 
	number_collection<K> * c,
	std::vector<approximated_curve> * approximated_curves)
{
	using internal_geometry::get_local_geometry;
	using internal_geometry::get_globalizer;
	typedef boost::format fmt;
		
	auto ss = m->spaces();
	std::vector<space_info> space_vector;
	*approximated_curves = std::vector<approximated_curve>();

	for (auto sp = ss.begin(); sp != ss.end(); ++sp) {
		std::string id;
		string_field(**sp, "GlobalId", &id);
		try {
			if (space_filter(id.c_str())) {
				fmt m("Extracting space %s...");
				msg_func(const_cast<char *>((m % id).str().c_str()));
				auto geometry = get_local_geometry(**sp, s, c);
				auto globalizer = get_globalizer(**sp, s, c);
				geometry->transform(globalizer);
				space_info this_space;
				strncpy(this_space.id, id.c_str(), SPACE_ID_MAX_LEN);
				this_space.geometry = geometry->to_interface_solid();
				space_vector.push_back(this_space);
				boost::copy(
					geometry->approximations(), 
					std::back_inserter(*approximated_curves));
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
	model * m, 
	size_t * element_count, 
	element_info *** elements, 
	double ** composite_layer_dxs,
	double ** composite_layer_dys,
	double ** composite_layer_dzs,
	size_t * space_count,
	space_info *** spaces,
	const unit_scaler & scaler,
	void (*notify)(char *),
	void (*warn)(char *),
	const std::function<bool(const char *)> & element_filter,
	const std::function<bool(const char *)> & space_filter,
	number_collection<K> * c,
	std::vector<element_info *> * shadings,
	std::vector<approximated_curve> * approximated_curves)
{
	std::vector<approximated_curve> element_approxes, space_approxes;
	char buf[256];
	*element_count = get_elements(
		m, 
		elements, 
		composite_layer_dxs,
		composite_layer_dys,
		composite_layer_dzs,
		notify, 
		warn,
		scaler, 
		element_filter, 
		c, 
		shadings,
		&element_approxes);
	sprintf(buf, "Got %u building elements.\n", *element_count);
	notify(buf);
	*space_count = get_spaces(
		m, 
		spaces, 
		notify, 
		warn, 
		scaler, 
		space_filter, 
		c, 
		&space_approxes);
	sprintf(buf, "Got %u building spaces.\n", *space_count);
	notify(buf);
	*approximated_curves = element_approxes;
	boost::copy(space_approxes, std::back_inserter(*approximated_curves));
	return IFCADAPT_OK;
}