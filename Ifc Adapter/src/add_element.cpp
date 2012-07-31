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

	if (type == WINDOW || type == DOOR) {
		exact_solid s;
		auto related_opening = get_related_opening(inst);
		if (related_opening) {
			ifc_to_solid(&s, *related_opening, scaler);
			s.populate_inexact_version(&info->geometry);
		}
		else {
			sprintf(buf, "Warning: door or window element does not have a related opening. It will be skipped.\n");
			msg_func(buf);
			return; 
		}
	}
	else {
		exact_solid s;
		ifc_to_solid(&s, inst, scaler);
		s.populate_inexact_version(&info->geometry);
	}

	infos->push_back(info);

	msg_func("done\n");
}