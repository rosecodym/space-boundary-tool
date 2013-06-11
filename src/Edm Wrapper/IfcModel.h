#pragma once

#include "EdmDatabase.h"

using namespace System;

namespace IfcInterface {

public ref class IfcModel {
public:
	IfcModel(String ^ pathToFile);

private:
	static IfcModel();

	static Lazy<EdmDatabase ^> ^ database;
};

} // namespace IfcInterface