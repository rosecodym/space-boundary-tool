#include "precompiled.h"

#include <gtest/gtest.h>

#include "area.h"

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

} // namespace

} // namespace geometry_2d