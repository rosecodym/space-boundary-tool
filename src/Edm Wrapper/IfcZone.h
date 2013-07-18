#pragma once

#include <cpp_edmi.h>

using namespace System;

namespace IfcInterface {

public ref class IfcZone
{
public:
	property String ^ Guid { String ^ get() { return guid; } }
	property String ^ Name { String ^ get() { return name; } }

internal:
	IfcZone(const cppw::Instance & inst) 
		: guid(gcnew String(((cppw::String)inst.get("GlobalId")).data()))
	{
		cppw::Select sel = inst.get("Name");
		if (sel.is_set()) {
			name = gcnew String(((cppw::String)sel).data());
		}
	}

private:
	initonly String ^ guid;
	initonly String ^ name;
};

} // namespace IfcInterface