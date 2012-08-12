#pragma once

using namespace System;

namespace IfcInformationExtractor {

public ref class MaterialLayer {
private:
	initonly String ^ name;
	initonly double thickness;
public:
	MaterialLayer(String ^ name, double thickness) : name(name), thickness(thickness) { }

	property String ^ Name
	{
		String ^ get() { return name; }
	}

	property double Thickness
	{
		double get() { return thickness; }
	}
};

} // namespace IfcInformationExtractor