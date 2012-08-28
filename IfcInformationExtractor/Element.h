#pragma once

#include <cpp_edmi.h>

#include "Construction.h"
#include "ConstructionFactory.h"

using namespace System;
using namespace System::Collections::Generic;

namespace IfcInformationExtractor {

public ref class Element {
private:
	initonly String ^ guid;
	initonly Construction ^ construction;
public:
	Element(const cppw::Instance & inst, ConstructionFactory ^ constructionFactory);

	property String ^ Guid
	{
		String ^ get() { return guid; }
	}

	property Construction ^ AssociatedConstruction
	{
		Construction ^ get() { return construction; }
	}
};

} // namespace IfcInformationExtractor