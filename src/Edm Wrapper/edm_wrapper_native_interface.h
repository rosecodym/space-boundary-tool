#pragma once

#include <exception>
#include <string>
#include <vector>
#include <boost/optional.hpp>

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
// model::invalidate_all_object_pointers.

EDM_WRAPPER_INTERFACE bool is_kind_of(
	const ifc_object & obj, 
	const std::string & type);
EDM_WRAPPER_INTERFACE bool is_instance_of(
	const ifc_object & obj, 
	const std::string & type);
EDM_WRAPPER_INTERFACE bool boolean_field(
	const ifc_object & obj,
	const std::string & field_name);
EDM_WRAPPER_INTERFACE std::vector<ifc_object *> collection_field(
	const ifc_object & obj,
	const std::string & field_name);
EDM_WRAPPER_INTERFACE ifc_object * object_field(
	const ifc_object & obj,
	const std::string & field_name);
EDM_WRAPPER_INTERFACE double real_field(
	const ifc_object & obj,
	const std::string & field_name);
EDM_WRAPPER_INTERFACE boost::optional<std::string> string_field(
	const ifc_object & obj, 
	const std::string & field_name);
EDM_WRAPPER_INTERFACE std::tuple<double, double, double> triple_field(
	const ifc_object & obj,
	const std::string & field_name);

class EDM_WRAPPER_INTERFACE model {
public:
	model(const char * path);
	
	const std::vector<ifc_object *> &	building_elements() const;
	void								invalidate_all_object_pointers();
	double								length_units_per_meter() const;
	void								remove_all_space_boundaries();
	void								write(const std::string & path) const;
};

} // namespace ifc_interface