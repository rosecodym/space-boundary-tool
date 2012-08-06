#pragma once

#include "precompiled.h"

#include "cgal-typedefs.h"

class number_collection;
class unit_scaler;

point_3				build_point(const cppw::Instance & inst, const unit_scaler & s, number_collection * c);
direction_3			build_direction(const cppw::Instance & inst, number_collection * c);
transformation_3	build_transformation(const cppw::Select & sel, const unit_scaler & s, number_collection * c);