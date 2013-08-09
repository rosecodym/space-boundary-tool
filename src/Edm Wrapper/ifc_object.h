#pragma once

#include <boost/variant.hpp>

#include <cpp_edmi.h>

namespace ifc_interface {

class model;

class ifc_object {
public:
	ifc_object(const cppw::Select & sel, model * m) : data_(sel), m_(m) { }
	
	cppw::Select data() const { return data_; }
	model * parent_model() const { return m_; }

private:
	cppw::Select data_;
	model * m_;
};

} // namespace ifc_object