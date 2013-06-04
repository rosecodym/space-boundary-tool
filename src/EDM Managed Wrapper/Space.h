#pragma once

#include <cpp_edmi.h>

#include "Zone.h"

using namespace System;
using namespace System::Collections::Generic;

namespace IfcInformationExtractor {

public ref class Space {
private:
	initonly String ^ guid;
	initonly String ^ name;
	initonly String ^ longName;
	initonly ICollection<Zone ^> ^ zones;
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

	property ICollection<Zone ^> ^ Zones
	{
		ICollection<Zone ^> ^ get() { return zones; }
	}
};

} // namespace IfcInformationExtractor