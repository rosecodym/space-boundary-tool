#pragma once

#include "Construction.h"

using namespace System;

namespace IfcInformationExtractor { 

public ref class SingleMaterial : Construction {
private:
	initonly String ^ name;
	initonly bool isForWindows;

public:
	SingleMaterial(String ^ name, bool forWindows) : name(name), isForWindows(forWindows) { }

	property String ^ Name { virtual String ^ get() override { return name; } }
	property bool IsComposite { virtual bool get() override { return false; } } 
	property bool IsForWindows { virtual bool get() override { return isForWindows; } }
	property String ^ Summary { virtual String ^ get() override { return Name; } }
	property String ^ Details { virtual String ^ get() override { return nullptr; } }
};

} // namespace IfcInformationExtractor