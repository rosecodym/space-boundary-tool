#include "precompiled.h"

#include "ifc-to-solid.h"
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

void add_element(std::vector<element_info *> * infos, element_type type, const cppw::Instance & inst, void (*msg_func)(char *), const unit_scaler & scaler) {
	static int next_construction_id = 1;

	char buf[256];

	sprintf(buf, "Extracting element %s...", ((cppw::String)inst.get("GlobalId")).data());
	msg_func(buf);

	element_info * info = (element_info *)malloc(sizeof(element_info));
	info->geometry.rep_type = REP_NOTHING;

	strncpy(info->id, ((cppw::String)inst.get("GlobalID")).data(), ELEMENT_ID_MAX_LEN);
	info->type = type;
	info->material = next_construction_id++;

	exact_solid s;
	int got_geometry = -1;

	if (type == WINDOW || type == DOOR) {
		auto related_opening = get_related_opening(inst);
		if (related_opening) {
			got_geometry = ifc_to_solid(&s, *related_opening, scaler);
		}
		else {
			msg_func("Warning: door or window element does not have a related opening.\n");
		}
	}
	else {
		got_geometry = ifc_to_solid(&s, inst, scaler);
	}

	if (got_geometry == 0) {
		s.populate_inexact_version(&info->geometry);
		infos->push_back(info);
		msg_func("done\n");
	}
	else {
		msg_func("This element will be skipped.\n");
	}
}