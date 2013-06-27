#include "IfcModel.h"

#include "EdmException.h"

namespace IfcInterface {

namespace {

double getMetersPerLengthUnit(const cppw::Instance & inst) {
	if (inst.is_kind_of("IfcSIUnit")) {
		cppw::Select sel = inst.get("Prefix");
		if (sel.is_set()) {
			cppw::String prefix = sel;
			if (prefix == "MEGA") { return 1e6; }
			else if (prefix == "KILO") { return 1e3; }
			else if (prefix == "HECTO") { return 1e2; }
			else if (prefix == "DECA") { return 1e1; }
			else if (prefix == "DECI") { return 1e-1; }
			else if (prefix == "CENTI") { return 1e-2; }
			else if (prefix == "MILLI") { return 1e-3; }
			else if (prefix == "MICRO") { return 1e-6; }
		}
	}
	else if (inst.is_kind_of("IfcConversionBasedUnit")) {
		cppw::Instance conversionFactor = inst.get("ConversionFactor");
		double factor = (cppw::Real)conversionFactor.get("ValueComponent");
		cppw::Instance unitComponent = conversionFactor.get("UnitComponent");
		return factor * getMetersPerLengthUnit(unitComponent);
	}
	return 1.0;	
}

}

IfcModel::IfcModel(String ^ path) 
	: database_(EdmDatabase::Instance()),
	  repo_(__nullptr),
	  model_(__nullptr)
{
	try { 
		model_ = database_->LoadModel(path); 
	}
	catch (cppw::Error & e) { throw gcnew EdmException(e.message.data()); }
}

double IfcModel::Elevation::get() {
	double res = 0.0;
	cppw::Instance site = model_->get_set_of("IfcSite").get_(0);
	cppw::Select sel = site.get("RefElevation");
	if (sel.is_set()) { res = (cppw::Real)sel; }
	return res;
}

double IfcModel::MetersPerLengthUnit::get() {
	cppw::Set projects = model_->get_set_of("IfcProject");
	cppw::Instance project = projects.get_(0);
	cppw::Instance unitAssignment = project.get("UnitsInContext");
	double res = 1.0;
	cppw::Set units = unitAssignment.get("Units");
	for (units.move_first(); units.move_next(); ) {
		cppw::Instance unit = units.get_();
		if (unit.is_kind_of("IfcNamedUnit")) {
			cppw::String type = unit.get("UnitType");
			if (type == "LENGTHUNIT") { res = getMetersPerLengthUnit(unit); }
		}
	}
	return res;
}

double IfcModel::NorthAxis::get() {
	// I don't know why this isn't implemented but nobody's complained yet!
	return 0.0;
}

double IfcModel::Latitude::get() {
	double res = 0.0;
	cppw::Instance site = model_->get_set_of("IfcSite").get_(0);
	cppw::Select sel = site.get("RefLatitude");
	if (sel.is_set()) {
		cppw::List lst = sel;
		res += (cppw::Real)lst.get_(0);
		res += (cppw::Real)lst.get_(1) / 60.0;
		res += (cppw::Real)lst.get_(2) / 360.0;
	}
	return res;
}

double IfcModel::Longitude::get() {
	double res = 0.0;
	cppw::Instance site = model_->get_set_of("IfcSite").get_(0);
	cppw::Select sel = site.get("RefLongitude");
	if (sel.is_set()) {
		cppw::List lst = sel;
		res += (cppw::Real)lst.get_(0);
		res += (cppw::Real)lst.get_(1) / 60.0;
		res += (cppw::Real)lst.get_(2) / 360.0;
	}
	return res;
}

ICollection<IfcElement ^> ^ IfcModel::Elements::get() {
	if (model_ == __nullptr) { return nullptr; }
	cppw::Instance_set instances = 
		model_->get_set_of("IfcBuildingElement", cppw::include_subtypes);
	IList<IfcElement ^> ^ elements = gcnew List<IfcElement ^>();
	for (instances.move_first(); instances.move_next(); ) {
		elements->Add(gcnew IfcElement(instances.get()));
	}
	return elements;
}

ICollection<IfcSpace ^> ^ IfcModel::Spaces::get() {
	if (model_ == __nullptr) { return nullptr; }
	cppw::Instance_set instances = 
		model_->get_set_of("IfcSpace", cppw::include_subtypes);
	List<IfcSpace ^> ^ spaces = gcnew List<IfcSpace ^>();
	for (instances.move_first(); instances.move_next(); ) {
		spaces->Add(gcnew IfcSpace(instances.get()));
	}
	return spaces;
}

} // namespace IfcInterface