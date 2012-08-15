#include "Space.h"

namespace IfcInformationExtractor {

Space::Space(const cppw::Instance & inst) {
	guid = gcnew String(((cppw::String)inst.get("GlobalId")).data());
	cppw::Select sel;
	if ((sel = inst.get("Name")).is_set()) {
		name = gcnew String(((cppw::String)sel).data());
	}
	if ((sel = inst.get("LongName")).is_set()) {
		longName = gcnew String(((cppw::String)sel).data());
	}
}

} // namespace IfcInformationExtractor