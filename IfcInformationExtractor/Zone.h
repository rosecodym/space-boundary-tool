#pragma once

#include <cpp_edmi.h>

using namespace System;

namespace IfcInformationExtractor {

public ref class Zone {
private:
	initonly String ^ guid;
	initonly String ^ name;
public:
	Zone(const cppw::Instance & inst) 
		: guid(gcnew String(((cppw::String)inst.get("GlobalId")).data()))
	{
		cppw::Select sel = inst.get("Name");
		if (sel.is_set()) {
			name = gcnew String(((cppw::String)sel).data());
		}
	}

	property String ^ Guid
	{
		String ^ get() { return guid; }
	}

	property String ^ Name
	{
		String ^ get() { return name; }
	}
};

} // namespace IfcInformationExtractor