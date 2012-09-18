#pragma once

#include "precompiled.h"

#include "cleanup_loop.h"

namespace geometry_2d {

namespace nef_polygons {

namespace util {

inline espoint_2 to_espoint(const point_2 & p) { return espoint_2(p.x(), p.y()); }

nef_polygon_2 create_nef_polygon(polygon_2 poly);

void snap(nef_polygon_2 * from, const nef_polygon_2 & to);

nef_polygon_2 clean(const nef_polygon_2 & nef);

} // namespace util

} // namespace nef_polygons 

} // namespace geometry_2d