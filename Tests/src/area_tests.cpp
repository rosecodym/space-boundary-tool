#include "precompiled.h"

#include <gtest/gtest.h>

#include "area.h"
#include "equality_context.h"
#include "sbt-core.h"

extern sb_calculation_options g_opts;

namespace geometry_2d {

namespace {

TEST(Area, StrictSubsetRectangles) {
	point_2 larger[] = {
		point_2(0, -300),
		point_2(8250, -300),
		point_2(8250, 0),
		point_2(0, 0)
	};
	point_2 smaller[] = {
		point_2(0, -300),
		point_2(4050, -300), 
		point_2(4050, 0),
		point_2(0, 0)
	};
	EXPECT_TRUE(area::do_intersect(
		area(polygon_2(larger, larger + 4)),
		area(polygon_2(smaller, smaller + 4))));
}

TEST(Area, IntersectionInvariability) {
	point_2 larger_pts[] = {
		point_2(0, -300),
		point_2(8250, -300),
		point_2(8250, 0),
		point_2(0, 0)
	};
	point_2 smaller_pts[] = {
		point_2(0, -300),
		point_2(4050, -300), 
		point_2(4050, 0),
		point_2(0, 0)
	};
	area larger(polygon_2(larger_pts, larger_pts + 4));
	area smaller(polygon_2(smaller_pts, smaller_pts + 4));
	EXPECT_EQ(
		area::do_intersect(larger, smaller),
		area::do_intersect(larger, smaller));
}

TEST(Area, SubtractionIsValidVertexOnDiagonal) {
	equality_context c(g_opts.equality_tolerance);
	point_2 op1[] = {
		point_2(27.443970, 8.339190),
		point_2(28.769285, 8.339190),
		point_2(28.769285, 8.844385),
		point_2(27.443970, 8.844385)
	};
	point_2 op2[] = {
		point_2(28.665582, 8.744685),
		point_2(28.883634, 8.954685),
		point_2(28.883634, 11.736685),
		point_2(28.665582, 11.736685)
	};
	for (size_t i = 0; i < 4; ++i) {
		// this is in the test because the contextualization is relevant
		c.snap(&op1[i]);
		c.snap(&op2[i]);
	}
	area res = area(polygon_2(op1, op1 + 4)) - area(polygon_2(op2, op2 + 4));
	EXPECT_TRUE(res.is_valid());
}

} // namespace

} // namespace geometry_2d