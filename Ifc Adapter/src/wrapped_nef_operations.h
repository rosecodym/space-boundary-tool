#pragma once

#include "precompiled.h"

struct exact_solid;
class unit_scaler;

namespace wrapped_nef_operations {

	void solid_from_boolean_result(exact_solid * s, const cppw::Instance & inst, const unit_scaler & scaler);

} // namespace wrapped_nef_operations