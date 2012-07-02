#pragma once

#include "precompiled.h"

#include "cgal-typedefs.h"

class unit_scaler;

point_3				build_point(const cppw::Instance & inst, const unit_scaler & s);
direction_3			build_direction(const cppw::Instance & inst);
transformation_3	build_transformation(const cppw::Select & sel, const unit_scaler & s);