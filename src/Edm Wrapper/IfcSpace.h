#pragma once

using namespace System;

namespace IfcInterface {

public ref class IfcSpace
{
public:
	IfcSpace() { throw gcnew NotImplementedException(); }

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
};

} // namespace IfcInterface