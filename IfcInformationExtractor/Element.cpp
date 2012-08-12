#include <cpp_edmi.h>

#include "Element.h"

namespace IfcInformationExtractor {

Element::Element(const cppw::Instance & inst)
	: guid(gcnew String(((cppw::String)inst.get("GlobalId")).data()))
{ }

} // namespace IfcInformationExtractor