#pragma once

#include <cpp_edmi.h>

#include "MaterialLayer.h"

using namespace System;
using namespace System::Collections::Generic;

namespace IfcInformationExtractor {

public ref class Element {
private:
	initonly String ^ guid;
	initonly IList<MaterialLayer ^> ^ construction;
public:
	Element(const cppw::Instance & inst);

	property String ^ Guid
	{
		String ^ get() { return guid; }
	}

	property IList<MaterialLayer ^> ^ Construction
	{
		IList<MaterialLayer ^> ^ get() { return construction; }
	}
};

} // namespace IfcInformationExtractor