#pragma once

#include <cpp_edmi.h>

using namespace System;
using namespace System::Collections::Generic;

namespace IfcInterface {

public ref class IfcElement
{
public:
	property String ^ Guid { String ^ get() { return guid; } }
	property bool IsShading { bool get() { return isShading; } }
	property bool IsWindow { bool get() { return isWindow; } }
	property String ^ ConstructionCompositeName
	{
		String ^ get() { return compositeName; }
	}
	property IList<String ^> ^ LayerNames
	{ 
		IList<String ^> ^ get() { return layerNames; }
	}
	property IList<double> ^ LayerThicknesses 
	{ 
		IList<double> ^ get() { return layerThicknesses; }
	}

internal:
	IfcElement(const cppw::Instance & inst);

private:
	String ^ compositeName;
	List<String ^> ^ layerNames;
	List<double> ^ layerThicknesses;
	String ^ guid;
	bool isShading;
	bool isWindow;
};

} // namespace IfcInterface