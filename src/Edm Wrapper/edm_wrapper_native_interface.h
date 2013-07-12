#pragma once

#include <exception>
#include <string>
#include <vector>

#ifdef EDM_WRAPPER_EXPORTS
#define EDM_WRAPPER_INTERFACE __declspec(dllexport)
#else
#define EDM_WRAPPER_INTERFACE __declspec(dllimport)
#endif

namespace ifc_interface {

class ifc_object;

// Some of these functions return pointers. They'll be freed when whatever
// model object they eventually descended from is freed. Alternatively, you
// can free (and invalidate) them manually by calling 
// model::invalidate_all_ownership_pointers.

EDM_WRAPPER_INTERFACE bool is_kind_of(
	const ifc_object & obj, 
	const char * type);
EDM_WRAPPER_INTERFACE bool is_instance_of(
	const ifc_object & obj, 
	const char * type);
EDM_WRAPPER_INTERFACE bool boolean_field(
	const ifc_object & obj,
	const char * field_name);
EDM_WRAPPER_INTERFACE std::vector<ifc_object *> collection_field(
	const ifc_object & obj,
	const char * field_name);
EDM_WRAPPER_INTERFACE ifc_object * object_field(
	const ifc_object & obj,
	const char * field_name);
EDM_WRAPPER_INTERFACE double real_field(
	const ifc_object & obj,
	const char * field_name);
EDM_WRAPPER_INTERFACE std::string string_field(
	const ifc_object & obj, 
	const char * field_name);
EDM_WRAPPER_INTERFACE bool triple_field(
	const ifc_object & obj,
	const char * field_name,
	double * a,
	double * b,
	double * c);

EDM_WRAPPER_INTERFACE bool set_field(
	ifc_object * obj,
	const char * field_name,
	const char * value);
EDM_WRAPPER_INTERFACE bool set_field(
	ifc_object * obj,
	const char * field_name,
	const ifc_object & value);

class EDM_WRAPPER_INTERFACE model {
public:
	model(const char * path);
	~model();
	
	std::vector<const ifc_object *> building_elements();
	std::vector<const ifc_object *> spaces();

	double length_units_per_meter() const;

	void write(const std::string & path) const;
	
	ifc_object * create_curve(
		const std::vector<std::pair<double, double>> & points);
	ifc_object * create_direction(double dx, double dy, double dz);
	ifc_object * create_object(
		const std::string & type,
		bool with_owner_history);
	ifc_object * create_point(double x, double y);
	ifc_object * create_point(double x, double y, double z);

	void invalidate_all_ownership_pointers();
	void remove_all_space_boundaries();
	bool set_new_owner_history(
		const char * app_full_name,
		const char * app_identifier,
		const char * app_version,
		const char * organization);

	// For internal use only
	ifc_object * take_ownership(ifc_object && obj);

private:
	struct internals;

	internals * d_;
};

} // namespace ifc_interface