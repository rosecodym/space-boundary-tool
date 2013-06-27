#pragma once

#include <cpp_edmi.h>

#include "IfcZone.h"

using namespace System;
using namespace System::Collections::Generic;

namespace IfcInterface {

public ref class IfcSpace
{
public:
	property String ^ Guid { String ^ get() { return guid; } }
	property String ^ LongName { String ^ get() { return longName; } }
	property String ^ Name { String ^ get() { return name; } }
	property ICollection<IfcZone ^> ^ Zones
	{
		ICollection<IfcZone ^> ^ get() { return zones; }
	}

internal:
	IfcSpace(const cppw::Instance & inst);

private:
	initonly String ^ guid;
	initonly String ^ name;
	initonly String ^ longName;
	initonly ICollection<IfcZone ^> ^ zones;
};

} // namespace IfcInterface