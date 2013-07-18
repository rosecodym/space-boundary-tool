#pragma once

#include <boost/variant.hpp>

#include <cpp_edmi.h>

namespace ifc_interface {

class model;

class ifc_object {
public:
	ifc_object(const cppw::Application_instance & app_inst, model * m)
		: data_(app_inst),
		  m_(m)
	{ }
	ifc_object(const cppw::Instance & inst, model * m) 
		: data_(inst),
		  m_(m)
	{ }

	const cppw::Instance * as_instance() const;
	cppw::Application_instance * as_app_instance();
	model * parent_model() const { return m_; }

private:
	boost::variant<cppw::Instance, cppw::Application_instance> data_;
	model * m_;
};

} // namespace ifc_object