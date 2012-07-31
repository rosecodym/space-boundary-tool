#include "precompiled.h"

#include "get_length_units_per_meter.h"

namespace {

double get_scale_for(const cppw::Instance & inst) {
	if (inst.is_kind_of("IfcSIUnit")) {
		cppw::Select sel = inst.get("Prefix");
		if (sel.is_set()) {
			cppw::String prefix = sel;
			if (prefix == "MEGA") {
				return 10e-6;
			}
			else if (prefix == "KILO") {
				return 10e-3;
			}
			else if (prefix == "HECTO") {
				return 10e-2;
			}
			else if (prefix == "DECA") {
				return 10e-1;
			}
			else if (prefix == "DECI") {
				return 10e1;
			}
			else if (prefix == "CENTI") {
				return 10e2;
			}
			else if (prefix == "MILLI") {
				return 10e3;
			}
			else if (prefix == "MICRO") {
				return 10e6;
			}
			else {
				return 1.0;
			}
		}
		return 1.0;
	}
	else if (inst.is_kind_of("IfcConversionBasedUnit")) {
		cppw::Instance conversionFactor = inst.get("ConversionFactor");
		return (1.0 / (double)(cppw::Real)conversionFactor.get("ValueComponent")) * get_scale_for((cppw::Instance)conversionFactor.get("UnitComponent"));
	}
	else {
		return 1.0;
	}
}

} // namespace

double get_length_units_per_meter(const cppw::Open_model & model) {
	cppw::Set projects = model.get_set_of("IfcProject");
	cppw::Instance project = projects.get_(0);
	cppw::Instance unitAssignment = project.get("UnitsInContext");

	cppw::Set units = unitAssignment.get("Units");
	for (units.move_first(); units.move_next(); ) {
		cppw::Instance unit = units.get_();
		if (unit.is_kind_of("IfcNamedUnit")) {
			cppw::String type = unit.get("UnitType");
			if (type == "LENGTHUNIT") {
				return get_scale_for(unit);
			}
		}
	}

	return 1.0;
}