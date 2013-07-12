#pragma once

#include <list>
#include <map>
#include <string>

#include <boost/optional.hpp>

#include "edm_wrapper_native_interface.h"

#include "ifc_object.h"

namespace cppw {
	
class Instance;
class Open_model;

} // namespace cppw

namespace ifc_interface {

struct model::internals {
	internals() : m(__nullptr) { }
	cppw::Open_model * m;
	std::list<ifc_object> known_objects;
	boost::optional<cppw::Instance> owner_history;
};

} // namespace ifc_interface