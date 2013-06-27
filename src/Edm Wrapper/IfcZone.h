#pragma once

using namespace System;

namespace IfcInterface {

public ref class IfcZone
{
public:
	property String ^ Guid
	{
		String ^ get() { throw gcnew NotImplementedException(); }
	}
	property String ^ Name
	{
		String ^ get() { throw gcnew NotImplementedException(); }
	}

internal:
	IfcZone() { throw gcnew NotImplementedException(); }
};

} // namespace IfcInterface