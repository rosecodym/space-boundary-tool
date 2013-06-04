#pragma once

#include "precompiled.h"

#include "polygon_with_holes_2.h"

namespace geometry_common {

point_2								flatten(const point_2 & p);
polygon_2							flatten(const polygon_2 & poly);
geometry_2d::polygon_with_holes_2	flatten(const polygon_with_holes_2 & pwh);

} // namespace geometry_common