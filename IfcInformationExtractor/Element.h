#pragma once

#include <cpp_edmi.h>

using namespace System;
using namespace System::Collections::Generic;

using namespace ConstructionManagement::ModelConstructions;

namespace IfcInformationExtractor {

public ref class Element {
private:
	initonly String ^ guid;
	initonly ModelConstruction ^ construction;
	initonly bool isShading;

public:
	Element(const cppw::Instance & inst, ModelConstructionCollection ^ constructions);

	property String ^ Guid
	{
		String ^ get() { return guid; }
	}

	property ModelConstruction ^ AssociatedConstruction
	{
		ModelConstruction ^ get() { return construction; }
	}

	property bool IsShading
	{
		bool get() { return isShading; }
	}
};

} // namespace IfcInformationExtractor