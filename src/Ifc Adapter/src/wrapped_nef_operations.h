#pragma once

#include "precompiled.h"

#include "internal_geometry.h"
#include "number_collection.h"

struct exact_solid;
class unit_scaler;

namespace ifc_interface {

class ifc_object;

} // namespace ifc_interface

namespace wrapped_nef_operations {

//void solid_from_boolean_result(exact_solid * s, const cppw::Instance & inst, const unit_scaler & scaler, number_collection<K> * c);
std::unique_ptr<internal_geometry::solid> from_boolean_result(
	const ifc_interface::ifc_object & obj,
	const unit_scaler & scaler,
	number_collection<K> * c);

} // namespace wrapped_nef_operations