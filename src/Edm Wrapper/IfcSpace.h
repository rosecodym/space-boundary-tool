#pragma once

using namespace System;

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

internal:
	IfcSpace() { throw gcnew NotImplementedException(); }
};

} // namespace IfcInterface