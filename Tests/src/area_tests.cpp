#include "precompiled.h"

#include <gtest/gtest.h>

#include "area.h"
#include "equality_context.h"

namespace geometry_2d {

namespace {

TEST(AreaDoIntersect, Subset) {
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
	// There was a bug once where the result wouldn't be consistent.
	EXPECT_TRUE(area::do_intersect(larger_a, smaller_a));
}

TEST(AreaConstruction, InvalidLoop) {
	point_2 pts[] = {
		point_2(27.443970, 8.339190),
		point_2(28.769285, 8.339190),
		point_2(28.769285, 8.844385),
		point_2(28.769105, 8.844385),
		point_2(28.665582, 8.744685),
		point_2(28.665582, 8.844385),
		point_2(27.443970, 8.844385)
	};
	polygon_2 poly(pts, pts + 7);
	EXPECT_FALSE(area(polygon_2(pts, pts + 7)).is_valid(0.01));
}

TEST(AreaConstructor, TwoFaces) {
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

TEST(AreaSubtraction, PointOnEdge) {
	// This is an angled quadrilateral "cutting into" a square through the
	// square's upper-right corner. I understand that the numbers are obnoxious
	// but if I make them simpler then the problem doesn't become apparent.
	point_2 rect_pts[] = {
		point_2(7.0, 8.339190),
		point_2(8.769285, 8.339190),
		point_2(8.769285, 8.844385),
		point_2(7.0, 8.844385)
	};
	point_2 nonrect_pts[] = {
		point_2(8.665582, 8.744685),
		point_2(8.883634, 8.954685),
		point_2(8.883634, 11.0),
		point_2(8.665582, 11.0)
	};
	area rect(polygon_2(rect_pts, rect_pts + 4));
	area nonrect(polygon_2(nonrect_pts, nonrect_pts + 4));
	area res = rect - nonrect;
	EXPECT_EQ(6, res.vertex_count());
}

TEST(AreaIntersection, Subset) {
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
	area a_smaller(polygon_2(smaller, smaller + 4));
	area a_larger(polygon_2(larger, larger + 4));
	area a_intr = a_smaller * a_larger;
	EXPECT_EQ(a_smaller, a_intr);
}

} // namespace

} // namespace geometry_2d