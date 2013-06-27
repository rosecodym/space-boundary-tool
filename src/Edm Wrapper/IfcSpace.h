#pragma once

#include "IfcZone.h"

using namespace System;
using namespace System::Collections::Generic;

namespace IfcInterface {

public ref class IfcSpace
{
public:
	property String ^ Guid 
	{ 
		String ^ get() { throw gcnew NotImplementedException(); }
	}
	property String ^ LongName 
	{ 
		String ^ get() { throw gcnew NotImplementedException(); } 
	}
	property String ^ Name 
	{ 
		String ^ get() { throw gcnew NotImplementedException(); } 
	}
	property ICollection<IfcZone ^> ^ Zones
	{
		ICollection<IfcZone ^> ^ get() { return gcnew List<IfcZone ^>(); }
	}

internal:
	IfcSpace() { throw gcnew NotImplementedException(); }
};

} // namespace IfcInterface