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

TEST(Area, NefRecontextualization) {
	equality_context c(g_opts.equality_tolerance);
	
	std::vector<polygon_2> faces;
	point_2 face_1[] = {
		point_2(-3.8, -0.2),
		point_2(0, -0.2),
		point_2(0, 0),
		point_2(-3.8, 0)
	};
	faces.push_back(polygon_2(face_1, face_1 + 4));

	point_2 face_2[] = {
		point_2(-3.8, 10),
		point_2(0, 10),
		point_2(0, 10.2),
		point_2(-3.8, 10.2)
	};
	faces.push_back(polygon_2(face_2, face_2 + 4));

	area dbl(faces);
	area dbl_rectxt(dbl, &c);

	SUCCEED();
}

} // namespace

} // namespace geometry_2d