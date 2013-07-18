#include <cpp_edmi.h>

#include "length_units_per_meter.h"

namespace IfcInterface {

namespace {

double length_units_per_meter(const cppw::Instance & unit) {
	if (unit.is_kind_of("IfcSIUnit")) {
		cppw::Select sel = unit.get("Prefix");
		if (sel.is_set()) {
			cppw::String prefix = sel;
			if (prefix == "MEGA") { return 1e-6; }
			else if (prefix == "KILO") { return 1e-3; }
			else if (prefix == "HECTO") { return 1e-2; }
			else if (prefix == "DECA") { return 1e-1; }
			else if (prefix == "DECI") { return 1e1; }
			else if (prefix == "CENTI") { return 1e2; }
			else if (prefix == "MILLI") { return 1e3; }
			else if (prefix == "MICRO") { return 1e6; }
		}
	}
	else if (unit.is_kind_of("IfcConversionBasedUnit")) {
		cppw::Instance conversionFactor = unit.get("ConversionFactor");
		double factor = (cppw::Real)conversionFactor.get("ValueComponent");
		cppw::Instance unitComponent = conversionFactor.get("UnitComponent");
		return factor * length_units_per_meter(unitComponent);
	}
	return 1.0;	
}

} // namespace

double length_units_per_meter(const cppw::Open_model & m) {
	cppw::Set projects = m.get_set_of("IfcProject");
	cppw::Instance project = projects.get_(0);
	cppw::Instance unitAssignment = project.get("UnitsInContext");
	double res = 1.0;
	cppw::Set units = unitAssignment.get("Units");
	for (units.move_first(); units.move_next(); ) {
		cppw::Instance unit = units.get_();
		if (unit.is_kind_of("IfcNamedUnit")) {
			cppw::String type = unit.get("UnitType");
			if (type == "LENGTHUNIT") { res = length_units_per_meter(unit); }
		}
	}
	return res;
}

} // namespace IfcInterface