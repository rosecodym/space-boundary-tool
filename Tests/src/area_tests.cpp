#include "precompiled.h"

#include <gtest/gtest.h>

#include "area.h"
#include "equality_context.h"

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
	area larger_a(polygon_2(larger, larger + 4));
	area smaller_a(polygon_2(smaller, smaller + 4));

	EXPECT_TRUE(area::do_intersect(larger_a, smaller_a));
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

TEST(Area, TwoFaces) {
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

	SUCCEED();
}

TEST(Area, Subtraction) {
	// two quadrilaterals
	// one of the rect vertices is on a diagonal edge of the non-rect
	equality_context c(0.01);
	point_2 rect_pts[] = {
		point_2(27.443970, 8.339190),
		point_2(28.769285, 8.339190),
		point_2(28.769285, 8.844385),
		point_2(27.443970, 8.844385)
	};
	point_2 nonrect_pts[] = {
		point_2(28.665582, 8.744685),
		point_2(28.883634, 8.954685),
		point_2(28.883634, 11.736685),
		point_2(28.665582, 11.736685)
	};
	for (size_t i = 0; i < 4; ++i) {
		// this is in the test because the contextualization is relevant
		c.snap(&rect_pts[i]);
		c.snap(&nonrect_pts[i]);
	}
	area rect(polygon_2(rect_pts, rect_pts + 4));
	area nonrect(polygon_2(nonrect_pts, nonrect_pts + 4));
	area res = rect - nonrect;
	EXPECT_EQ(6, res.vertex_count());
}

TEST(Area, LargerIntersection) {
	point_2 smaller[] = {
		point_2(0, 0),
		point_2(393, 0),
		point_2(393, 387),
		point_2(0, 387)
	};
	point_2 larger[] = {
		point_2(0, -8),
		point_2(803, -8),
		point_2(803, 401),
		point_2(0, 401)
	};
	EXPECT_EQ(area(polygon_2(smaller, smaller + 4)), area(polygon_2(smaller, smaller + 4)) * area(polygon_2(larger, larger + 4)));
}

} // namespace

} // namespace geometry_2d