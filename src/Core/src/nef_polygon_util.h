#pragma once

#include "precompiled.h"

#include "cleanup_loop.h"

namespace geometry_2d {

namespace nef_polygons {

namespace util {

nef_polygon_2	clean(const nef_polygon_2 & nef);
nef_polygon_2	create_nef_polygon(polygon_2 poly);
espoint_2		to_espoint(const point_2 & p);
size_t			vertex_count(const nef_polygon_2 & nef);

} // namespace util

} // namespace nef_polygons 

} // namespace geometry_2d