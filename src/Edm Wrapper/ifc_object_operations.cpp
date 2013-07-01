#include "edm_wrapper_native_interface.h"

#include "ifc_object.h"

namespace ifc_interface {

bool is_kind_of(const ifc_object & obj, const char * type) {
	return obj.as_instance()->is_kind_of(type);
}

bool is_instance_of(const ifc_object & obj, const char * type) {
	return obj.as_instance()->is_instance_of(type);
}

bool boolean_field(const ifc_object & obj, const char * field_name) {
	return static_cast<bool>(obj.as_instance()->get(field_name));
}

std::vector<ifc_object *> collection_field(
	const ifc_object & obj, 
	const char * field_name)
{
	cppw::Aggregate col = obj.as_instance()->get(field_name);
	std::vector<ifc_object *> res;
	model * m = obj.parent_model();
	for (col.move_first(); col.move_next(); ) {
		cppw::Instance inst = col.get_();
		res.push_back(m->take_ownership(ifc_object(inst, m)));
	}
	return res;
}

ifc_object * object_field(const ifc_object & obj, const char * field_name) {
	try {
		cppw::Select sel = obj.as_instance()->get(field_name);
		if (!sel.is_set()) { return __nullptr; }
		cppw::Instance inst = sel;
		model * m = obj.parent_model();
		return m->take_ownership(ifc_object(inst, m));
	}
	catch (...) { return __nullptr; }
}

double real_field(const ifc_object & obj, const char * field_name) {
	return static_cast<double>(obj.as_instance()->get(field_name));
}

std::string string_field(const ifc_object & obj, const char * field_name) {
	cppw::String res = obj.as_instance()->get(field_name);
	return std::string(res.data());
}

bool triple_field(
	const ifc_object & obj,
	const char * field_name,
	double * a,
	double * b,
	double * c)
{
	try {
		cppw::Aggregate col = obj.as_instance()->get(field_name);
		*a = static_cast<double>(col.get_(0));
		*b = static_cast<double>(col.get_(1));
		*c = col.size() > 2 ? static_cast<double>(col.get_(2)) : 0.0;
		return true;
	}
	catch (...) { return false; }
}
	
bool set_field(
	ifc_object * obj,
	const char * field_name,
	const char * value)
{
	try {
		cppw::Application_instance * app_inst = obj->as_app_instance();
		if (!app_inst) { return false; }
		app_inst->put(field_name, value);
		return true;
	}
	catch (...) { return false; }
}

bool set_field(
	ifc_object * obj,
	const char * field_name,
	const ifc_object & value)
{
	try {
		cppw::Application_instance * app_inst = obj->as_app_instance();
		const cppw::Instance * val = value.as_instance();
		if (!app_inst) { return false; }
		app_inst->put(field_name, *val);
		return true;
	}
	catch (...) { return false; }
}

} // namespace ifc_interface