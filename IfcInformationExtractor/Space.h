#pragma once

#include <cpp_edmi.h>

using namespace System;

namespace IfcInformationExtractor {

public ref class Space {
private:
	initonly String ^ guid;
	initonly String ^ name;
	initonly String ^ longName;
public:
	Space(const cppw::Instance & inst);

	property String ^ Guid
	{
		String ^ get() { return guid; }
	}

	property String ^ Name
	{
		String ^ get() { return name; }
	}

	property String ^ LongName
	{
		String ^ get() { return longName; }
	}
};

} // namespace IfcInformationExtractor