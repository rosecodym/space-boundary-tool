#include "edm_wrapper_native_interface.h"

#include "ifc_object.h"

using namespace cppw;

namespace ifc_interface {

bool is_kind_of(const ifc_object & obj, const char * type) {
	if (obj.data().get_primitive_type() != instance_type) { return false; }
	try { return Instance(obj.data()).is_kind_of(type); }
	catch (...) { return false; }
}

bool is_instance_of(const ifc_object & obj, const char * type) {
	if (obj.data().get_primitive_type() != instance_type) { return false; }
	try { return Instance(obj.data()).is_instance_of(type); }
	catch (...) { return false; }
}

bool boolean_field(
	const ifc_object & obj, 
	const char * field_name,
	bool * res)
{
	try { 
		if (obj.data().get_primitive_type() != instance_type) { return false; }
		Select sel = Instance(obj.data()).get(field_name);
		if (sel.get_primitive_type() != boolean_type) { return false; }
		*res = (Boolean)sel;
		return true;
	}
	catch (...) { return false; }
}

bool collection_field(
	const ifc_object & obj, 
	const char * field_name,
	std::vector<ifc_object *> * res)
{
	try {
		if (obj.data().get_primitive_type() != instance_type) { return false; }
		Select sel = Instance(obj.data()).get(field_name);
		if (sel.get_primitive_type() != aggregate_type) { return false; }
		Aggregate col(sel);
		model * m = obj.parent_model();
		assert(col.size() > 0);
		*res = std::vector<ifc_object *>();
		for (col.move_first(); col.move_next(); ) {
			res->push_back(m->take_ownership(ifc_object(col.get_(), m)));
		}
		return true;
	}
	catch (...) { return false; }
}

bool object_field(
	const ifc_object & obj, 
	const char * field_name,
	ifc_object ** res)
{
	try {
		if (obj.data().get_primitive_type() != instance_type) { return false; }
		Select sel = Instance(obj.data()).get(field_name);
		if (sel.is_set()) { 
			model * m = obj.parent_model();
			*res = m->take_ownership(ifc_object(sel, m));
		}
		else { *res = __nullptr; }
		return true;
	}
	catch (...) { return false; }
}

bool real_field(
	const ifc_object & obj, 
	const char * field_name, 
	double * res) 
{
	try {
		if (obj.data().get_primitive_type() != instance_type) { return false; }
		Select sel = Instance(obj.data()).get(field_name);
		if (sel.get_primitive_type() != real_type) { return false; }
		*res = (Real)sel;
		return true;
	}
	catch (...) { return false; }
}

bool string_field(
	const ifc_object & obj, 
	const char * field_name,
	std::string * res)
{
	try {
		if (obj.data().get_primitive_type() != instance_type) { return false; }
		Select sel = Instance(obj.data()).get(field_name);
		if (sel.get_primitive_type() != string_type) { return false; }
		*res = std::string(String(sel).data());
		return true;
	}
	catch (...) { return false; }
}

bool triple_field(
	const ifc_object & obj,
	const char * field_name,
	double * a,
	double * b,
	double * c)
{
	try {
		if (obj.data().get_primitive_type() != instance_type) { return false; }
		Select sel = Instance(obj.data()).get(field_name);
		if (sel.get_primitive_type() != aggregate_type) { return false; }
		Aggregate col(sel);
		sel = col.get_(0);
		if (sel.get_primitive_type() != real_type) { return false; }
		*a = (Real)sel;
		sel = col.get_(1);
		if (sel.get_primitive_type() != real_type) { return false; }
		*b = (Real)sel;
		if (col.size() > 2) {
			sel = col.get_(2);
			if (sel.get_primitive_type() != real_type) { return false; }
			*c = (Real)sel;
		}
		else { *c = 0.0; }
		return true;
	}
	catch (...) { return false; }
}

bool as_real(const ifc_object & obj, double * res) {
	try {
		if (obj.data().get_primitive_type() != real_type) { return false; }
		*res = (Real)obj.data();
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
		if (obj->data().get_primitive_type() != instance_type) { return false; }
		Application_instance(obj->data()).put(field_name, value);
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
		if (obj->data().get_primitive_type() != instance_type) { return false; }
		Application_instance(obj->data()).put(field_name, value.data());
		return true;
	}
	catch (...) { return false; }
}

} // namespace ifc_interface