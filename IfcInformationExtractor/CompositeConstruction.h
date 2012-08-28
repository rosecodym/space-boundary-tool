#pragma once

#include <cpp_edmi.h>

#include "Construction.h"

using namespace System;
using namespace System::Collections::Generic;

namespace IfcInformationExtractor {

public ref class CompositeConstruction : Construction {
private:
	initonly String ^ name;
public:
	CompositeConstruction(String ^ name) : name(name) { }

	property String ^ Name { virtual String ^ get() override { return name; } }
	property bool IsComposite { virtual bool get() override { return true; } }
	property String ^ Summary { virtual String ^ get() override { return Name; } }
	property String ^ Details { virtual String ^ get() override { return nullptr; } }
};

} // namespace IfcInformationExtractor