#pragma once

#include "Construction.h"
#include "Element.h"
#include "Space.h"

using namespace System;
using namespace System::Collections::Generic;

namespace IfcInformationExtractor {

public ref class BuildingInformation {
public:
	BuildingInformation() { }

	property String ^ Filename;

	property double NorthAxis;
	property double Latitude;
	property double Longitude;
	property double Elevation;

	property IDictionary<String ^, Space ^> ^ SpacesByGuid;
	property IDictionary<String ^, Element ^> ^ ElementsByGuid;
	property IEnumerable<Construction ^> ^ Constructions;
};

} // namespace IfcInformationExtractor