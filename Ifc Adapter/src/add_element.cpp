#include "precompiled.h"

#include "add_element.h"

#include "cgal-typedefs.h"
#include "internal_geometry.h"
#include "sbt-ifcadapter.h"

namespace {

boost::optional<cppw::Instance> get_related_opening(const cppw::Instance & fen_inst) {
	cppw::Set fillsVoids = fen_inst.get("FillsVoids");
	if (fillsVoids.count() == 1) {
		return (cppw::Instance)((cppw::Instance)fillsVoids.get_(0)).get("RelatingOpeningElement");
	}
	else {
		return boost::optional<cppw::Instance>();
	}
}

boost::optional<direction_3> get_composite_dir(
	const cppw::Instance & inst,
	const direction_3 & axis1,
	const direction_3 & axis2,
	const direction_3 & axis3) 
{
	if (!inst.is_kind_of("IfcWindow")) {
		cppw::Set relAssociates = inst.get("HasAssociations");
		for (relAssociates.move_first(); relAssociates.move_next(); ) {
			cppw::Instance rel = relAssociates.get_();
			if (rel.is_kind_of("IfcRelAssociatesMaterial")) {
				cppw::Instance relatingMat = rel.get("RelatingMaterial");
				if (relatingMat.is_kind_of("IfcMaterialLayerSetUsage")) {
					cppw::String dir = relatingMat.get("LayerSetDirection");
					cppw::String sense = relatingMat.get("DirectionSense");
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
	return boost::optional<direction_3>();
}

} // namespace

void add_element(
	std::vector<element_info *> * infos, 
	std::vector<direction_3> * composite_dirs,
	element_type type, 
	const cppw::Instance & inst, 
	void (*msg_func)(char *), 
	void (*warn_func)(char *),
	const unit_scaler & scaler, 
	int material_id, 
	number_collection<K> * c) 
{
	char buf[256];

	sprintf(buf, "Extracting element %s...", ((cppw::String)inst.get("GlobalId")).data());
	msg_func(buf);

	element_info * info = (element_info *)malloc(sizeof(element_info));
	info->geometry.rep_type = REP_NOTHING;

	strncpy(info->id, ((cppw::String)inst.get("GlobalID")).data(), ELEMENT_ID_MAX_LEN);
	info->type = type;
	info->material = material_id;

	boost::optional<cppw::Instance> effective_instance = inst;
	if (type == WINDOW || type == DOOR) {
		effective_instance = get_related_opening(inst);
		if (!effective_instance) {
			warn_func(
				"Warning: door or window element does not have a related "
				"opening. This element will be skipped.\n");
			return;
		}
	}
	
	try {
		auto geometry = internal_geometry::get_local_geometry(
			*effective_instance,
			scaler,
			c);
		auto globalizer = internal_geometry::get_globalizer(
			*effective_instance,
			scaler,
			c);
		geometry->transform(globalizer);
		info->geometry = geometry->to_interface_solid();
		infos->push_back(info);
		if (composite_dirs) {
			if (geometry->axis1()) {
				auto composite_dir = get_composite_dir(
					inst,
					*geometry->axis1(),
					*geometry->axis2(),
					*geometry->axis3());
				if (composite_dir) {
					composite_dirs->push_back(*composite_dir);
				}
				else {
					composite_dirs->push_back(direction_3());
				}
			}
			else {
				composite_dirs->push_back(direction_3());
			}
		}
		msg_func("done\n");
	}
	catch (internal_geometry::bad_rep_exception & ex) {
		sprintf(
			buf, 
			"Element %s could not be loaded (%s). It will be skipped.\n",
			info->id,
			ex.what());
		warn_func(buf);
	}
}