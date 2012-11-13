#include "Space.h"

namespace IfcInformationExtractor {

Space::Space(const cppw::Instance & inst)
	: zones(gcnew List<Zone ^>())
{
	guid = gcnew String(((cppw::String)inst.get("GlobalId")).data());
	cppw::Select sel;
	if ((sel = inst.get("Name")).is_set()) {
		name = gcnew String(((cppw::String)sel).data());
	}
	if ((sel = inst.get("LongName")).is_set()) {
		longName = gcnew String(((cppw::String)sel).data());
	}
	cppw::Set hasAssignments = inst.get("HasAssignments");
	for (hasAssignments.move_first(); hasAssignments.move_next(); ) {
		cppw::Instance assignment = hasAssignments.get_();
		if (assignment.is_instance_of("IfcRelAssignsToGroup")) {
			cppw::Instance group = assignment.get("RelatingGroup");
			if (group.is_kind_of("IfcZone")) {
				cppw::String id = group.get("GlobalId");
				zones->Add(gcnew Zone(group));
			}
		}
	}
}

} // namespace IfcInformationExtractor