#pragma once

#include "Element.h"

using namespace System;
using namespace System::Collections::Generic;

namespace IfcInformationExtractor {

public ref class BuildingInformation {
public:
	BuildingInformation() { }

	property ICollection<Element ^> ^ Elements;
};

} // namespace IfcInformationExtractor