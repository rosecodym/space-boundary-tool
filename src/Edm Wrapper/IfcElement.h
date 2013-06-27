#pragma once

using namespace System;

namespace IfcInterface {

public ref class IfcElement
{
public:
	property String ^ Guid 
	{ 
		String ^ get() { throw gcnew NotImplementedException(); } 
	}
	property bool IsShading
	{
		bool get() { throw gcnew NotImplementedException(); }
	}
	property Object ^ AssociatedConstruction
	{
		Object ^ get() { throw gcnew NotImplementedException(); }
	}

internal:
	IfcElement() { throw gcnew NotImplementedException(); }
};

} // namespace IfcInterface