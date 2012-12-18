#include "precompiled.h"

#include "internal_geometry.h"
#include "sbt-ifcadapter.h"
#include "add_element.h"

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

} // namespace

void add_element(std::vector<element_info *> * infos, element_type type, const cppw::Instance & inst, void (*msg_func)(char *), const unit_scaler & scaler, int material_id, number_collection<K> * c) {
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
			msg_func(
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
		msg_func("done\n");
	}
	catch (internal_geometry::bad_rep_exception & ex) {
		sprintf(
			buf, 
			"Warning: could not load this element (%s). It will be "
			"skipped.\n",
			ex.what());
		msg_func(buf);
	}
}