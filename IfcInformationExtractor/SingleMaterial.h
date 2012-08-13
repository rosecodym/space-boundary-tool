#pragma once

#include "Construction.h"

using namespace System;

namespace IfcInformationExtractor { 

public ref class SingleMaterial : Construction {
private:
	initonly String ^ name;
public:
	SingleMaterial(String ^ name) : name(name) { }

	property String ^ Name { virtual String ^ get() override { return name; } }
	property bool IsComposite { virtual bool get() override { return false; } } 
};

} // namespace IfcInformationExtractor