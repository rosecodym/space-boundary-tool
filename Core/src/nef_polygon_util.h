#pragma once

#include "precompiled.h"

#include "cleanup_loop.h"
#include "sbt-core.h"

extern sb_calculation_options g_opts;

namespace geometry_2d {

namespace nef_polygons {

namespace util {

inline espoint_2 to_espoint(const point_2 & p) { return espoint_2(p.x(), p.y()); }

inline nef_polygon_2 create_nef_polygon(polygon_2 poly) {
	if (!geometry_common::cleanup_loop(&poly, g_opts.equality_tolerance)) {
		return nef_polygon_2::EMPTY;
	}
	std::vector<espoint_2> extended;
	std::transform(poly.vertices_begin(), poly.vertices_begin(), std::back_inserter(extended), &to_espoint);
	return poly.is_counterclockwise_oriented() ?
		nef_polygon_2(extended.begin(), extended.end(), nef_polygon_2::EXCLUDED) :
		nef_polygon_2(extended.rbegin(), extended.rend(), nef_polygon_2::EXCLUDED);
}

void snap(nef_polygon_2 * from, const nef_polygon_2 & to);

nef_polygon_2 clean(const nef_polygon_2 & nef);

} // namespace util

} // namespace nef_polygons 

} // namespace geometry_2d