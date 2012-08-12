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

public ref class BuildingInformation {
private:
	initonly String ^ filename;
public:
	BuildingInformation(String ^ filename) : filename(filename) { }

	property String ^ Filename
	{
		String ^ get() { return filename; }
	}
};

} // namespace IfcInformationExtractor