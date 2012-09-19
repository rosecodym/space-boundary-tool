#include "precompiled.h"

#include <gtest/gtest.h>

#include "nef_polygon_util.h"

namespace geometry_2d {

namespace nef_polygons {

namespace util {

namespace {

nef_polygon_2 create_naive_nef_polygon(const polygon_2 & poly) {
	std::vector<espoint_2> extended;
	std::transform(poly.vertices_begin(), poly.vertices_end(), std::back_inserter(extended), [](const point_2 & p) { 
		return to_espoint(p); 
	});
	return poly.is_counterclockwise_oriented() ?
		nef_polygon_2(extended.begin(), extended.end(), nef_polygon_2::EXCLUDED) :
		nef_polygon_2(extended.rbegin(), extended.rend(), nef_polygon_2::EXCLUDED);
}

TEST(NefPolygonUtil, Clean) {
	// short edge
	point_2 pts[] = {
		point_2(27.443970, 8.339190),
		point_2(28.769285, 8.339190),
		point_2(28.769285, 8.844385),
		point_2(28.769105, 8.844385),
		point_2(28.665582, 8.744685),
		point_2(28.665582, 8.844385),
		point_2(27.443970, 8.844385)
	};
	nef_polygon_2 naive_nef = create_naive_nef_polygon(polygon_2(pts, pts + 7));
	ASSERT_EQ(7, vertex_count(naive_nef));
	nef_polygon_2 cleaned = clean(naive_nef);
	EXPECT_EQ(6, vertex_count(cleaned));
}

} // namespace

} // namespace util

} // namespace nef_polygons

} // namespace geometry_2d