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

std::unique_ptr<internal_geometry::solid> from_boolean_result(
	const ifc_interface::ifc_object & obj,
	const unit_scaler & scaler,
	number_collection<K> * c,
	std::function<void(char *)> notify_func);

} // namespace wrapped_nef_operations