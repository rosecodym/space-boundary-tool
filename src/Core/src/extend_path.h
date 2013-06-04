#pragma once

#include "precompiled.h"

#include "transmission_information.h"

namespace geometry_2d {
class area;
} // namespace geometry_2d

class orientation;

namespace traversal {

namespace impl {

class bg_path;

void extend_path(
	const bg_path & so_far, 
	const geometry_2d::area & transmission_area,
	const orientation * o,
	double max_thickness,
	const std::function<void(transmission_information)> & save_traversal);

} // namespace impl

} // namespace traversal