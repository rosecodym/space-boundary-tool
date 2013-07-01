#include "IfcModel.h"

#include "EdmException.h"
#include "utility.h"

namespace IfcInterface {

IfcModel::IfcModel(String ^ path) 
	: repo_(__nullptr),
	  model_(__nullptr)
{
	try { 
		model_ = EdmDatabase::Instance()->LoadModel(path); 
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

double IfcModel::LengthUnitsPerMeter::get() {
	return Internal::length_units_per_meter(*model_);
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