#include "IfcModel.h"

#include "EdmException.h"

namespace IfcInterface {

IfcModel::IfcModel(String ^ path) 
	: database_(EdmDatabase::Instance()),
	  repo_(__nullptr),
	  model_(__nullptr)
{
	try { model_ = database_->LoadModel(path); }
	catch (cppw::Error & e) { throw gcnew EdmException(e.message.data()); }
}

double IfcModel::Elevation::get() {
	double res = 0.0;
	cppw::Instance site = model_->get_set_of("IfcSite").get_(0);
	cppw::Select sel = site.get("RefElevation");
	if (sel.is_set()) { res = (cppw::Real)sel; }
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

} // namespace IfcInterface