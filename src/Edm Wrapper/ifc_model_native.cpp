#include <ctime>
#include <list>
#include <map>

#include <boost/optional.hpp>
#include <boost/variant.hpp>

#include <cpp_edmi.h>

#include "edm_wrapper_native_interface.h"

#include "ifc_model_internals.h"
#include "ifc_object.h"
#include "length_units_per_meter.h"

namespace ifc_interface {

model::~model() { 
	delete d_->m; 
	delete d_;
}

bool model::loaded_ok() const { return d_->m != __nullptr; }

const std::string & model::last_error() const { return d_->last_error; }

std::vector<const ifc_object *> model::building_elements() {
	std::vector<const ifc_object *> res;
	cppw::Instance_set elems = 
		d_->m->get_set_of("IfcBuildingElement", cppw::include_subtypes);
	for (elems.move_first(); elems.move_next(); ) {
		cppw::Instance inst = elems.get();
		d_->known_objects.push_back(ifc_object(inst, this));
		res.push_back(&d_->known_objects.back());
	}
	return res;
}

std::vector<const ifc_object *> model::spaces() {
	std::vector<const ifc_object *> res;
	cppw::Instance_set elems = 
		d_->m->get_set_of("IfcSpace", cppw::include_subtypes);
	for (elems.move_first(); elems.move_next(); ) {
		cppw::Instance inst = elems.get();
		d_->known_objects.push_back(ifc_object(inst, this));
		res.push_back(&d_->known_objects.back());
	}
	return res;
}

double model::length_units_per_meter() const {
	return IfcInterface::length_units_per_meter(*d_->m);
}

void model::write(const std::string & path) const {
	cppw::Step_writer(d_->m->get_model(), path.c_str()).write();
}

ifc_object * model::create_curve(
	const std::vector<std::pair<double, double>> & points)
{
	cppw::Application_instance curve = d_->m->create("IfcPolyline");
    cppw::List pts = curve.create_aggregate("Points");
    for (auto p = points.begin(); p != points.end(); ++p) {
		ifc_object * pt = this->create_point(p->first, p->second);
		assert(pt);
		cppw::Application_instance app_inst(pt->data());
		pts.add(app_inst);
    }
    d_->known_objects.push_back(ifc_object(curve, this));
	return &d_->known_objects.back();
}

ifc_object * model::create_direction(double dx, double dy, double dz) {
	cppw::Application_instance direction = d_->m->create("IfcDirection");
    cppw::List ratios(direction.create_aggregate("DirectionRatios"));
    ratios.add(dx);
	ratios.add(dy);
	ratios.add(dz);
	d_->known_objects.push_back(ifc_object(direction, this));
	return &d_->known_objects.back();
}

ifc_object * model::create_object(
	const std::string & type,
	bool with_owner_history)
{
	if (with_owner_history && !d_->owner_history) { return __nullptr; }
	try {
		cppw::Application_instance obj = d_->m->create(type.c_str());
		if (with_owner_history) { 
			obj.put("OwnerHistory", *d_->owner_history);
		}
		d_->known_objects.push_back(ifc_object(obj, this));
		return &d_->known_objects.back();
	}
	catch (...) { return __nullptr; }
}

ifc_object * model::create_point(double x, double y) {
    cppw::Application_instance point = d_->m->create("IfcCartesianPoint");
    cppw::List coords(point.create_aggregate("Coordinates"));
    coords.add(x);
	coords.add(y);
    d_->known_objects.push_back(ifc_object(point, this));
	return &d_->known_objects.back();
}

ifc_object * model::create_point(double x, double y, double z) {
    cppw::Application_instance point = d_->m->create("IfcCartesianPoint");
    cppw::List coords(point.create_aggregate("Coordinates"));
    coords.add(x);
	coords.add(y);
	coords.add(z);
    d_->known_objects.push_back(ifc_object(point, this));
	return &d_->known_objects.back();
}

void model::invalidate_all_ownership_pointers() {
	d_->known_objects.clear();
}

void model::remove_all_space_boundaries() {
	cppw::Application_aggregate sbs = 
		d_->m->get_set_of("IfcRelSpaceBoundary", cppw::include_subtypes);
	for (int i = 0; i < sbs.count(); ++i) {
		((cppw::Application_instance)sbs.get_()).remove();
	}
}

bool model::set_new_owner_history(
	const char * app_full_name,
	const char * app_identifier,
	const char * app_version,
	const char * organization)
{
	try {
		cppw::Application_aggregate histories = 
			d_->m->get_set_of("IfcOwnerHistory");
		if (histories.size() < 1) { return false; }
		cppw::Application_instance existing = histories.get_(0);
		cppw::Application_instance inst = existing.clone();
		inst.put("ChangeAction", "ADDED");
        cppw::Application_instance app = d_->m->create("IfcApplication");
        cppw::Application_instance org = d_->m->create("IfcOrganization");
        org.put("Name", organization);
        app.put("ApplicationDeveloper", org);
        app.put("Version", app_version);
        app.put("ApplicationFullName", app_full_name);
        app.put("ApplicationIdentifier", app_identifier);
        inst.put("LastModifyingApplication", app);
        inst.put("CreationDate", (int)time(NULL));
		d_->owner_history = inst;
		return true;
	}
	catch (cppw::Error & /*ex*/) { 
		return false; 
	}
}

ifc_object * model::take_ownership(ifc_object && obj) {
	d_->known_objects.push_back(obj);
	return &d_->known_objects.back();
}

} // namespace ifc_model