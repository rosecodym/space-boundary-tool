#include "precompiled.h"

#include "unit_scaler.h"

const unit_scaler unit_scaler::identity_scaler = unit_scaler();

namespace {

double get_scale_for(const cppw::Instance & inst) {
	if (inst.is_kind_of("IfcSIUnit")) {
		cppw::Select sel = inst.get("Prefix");
		if (sel.is_set()) {
			cppw::String prefix = sel;
			if (prefix == "MEGA") {
				return 1e6;
			}
			else if (prefix == "KILO") {
				return 1e3;
			}
			else if (prefix == "HECTO") {
				return 1e2;
			}
			else if (prefix == "DECA") {
				return 1e1;
			}
			else if (prefix == "DECI") {
				return 1e-1;
			}
			else if (prefix == "CENTI") {
				return 1e-2;
			}
			else if (prefix == "MILLI") {
				return 1e-3;
			}
			else if (prefix == "MICRO") {
				return 1e-6;
			}
			else {
				return 1.0;
			}
		}
		return 1;
	}
	else if (inst.is_kind_of("IfcConversionBasedUnit")) {
		cppw::Instance conversionFactor = inst.get("ConversionFactor");
		return (double)(cppw::Real)conversionFactor.get("ValueComponent") * get_scale_for((cppw::Instance)conversionFactor.get("UnitComponent"));
	}
	else {
		return 1.0;
	}
}

} // namespace

unit_scaler::unit_scaler(const cppw::Open_model & model) {
	cppw::Set projects = model.get_set_of("IfcProject");
	cppw::Instance project = projects.get_(0);
	cppw::Instance unitAssignment = project.get("UnitsInContext");

	length_factor = 1.0;

	cppw::Set units = unitAssignment.get("Units");
	for (units.move_first(); units.move_next(); ) {
		cppw::Instance unit = units.get_();
		if (unit.is_kind_of("IfcNamedUnit")) {
			cppw::String type = unit.get("UnitType");
			if (type == "LENGTHUNIT") {
				length_factor = get_scale_for(unit);
			}
		}
	}
}