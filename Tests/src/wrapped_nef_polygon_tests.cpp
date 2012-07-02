#include "precompiled.h"

#include <gtest/gtest.h>

#include "wrapped_nef_polygon.h"

namespace geometry_2d {

namespace {

TEST(WrappedNefPolygon, AxisAlignment) {
	point_2 pts1[] = {
		point_2(17182.249, 0),
		point_2(17182.249, -300),
		point_2(18195.109, -300),
		point_2(18195.109, 0)
	};
	wrapped_nef_polygon wrapped_1(polygon_2(pts1, pts1 + 4));
	EXPECT_TRUE(wrapped_1.is_axis_aligned());
	point_2 pts2[] = {
		point_2(12120.109, -300),
		point_2(18195.109, -300),
		point_2(18195.109, 0),
		point_2(12120.109, 0)
	};
	wrapped_nef_polygon wrapped_2(polygon_2(pts2, pts2 + 4));
	EXPECT_TRUE(wrapped_2.is_axis_aligned());
}

TEST(WrappedNefPolygon, IntersectionInvariability) {
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
	wrapped_nef_polygon larger(polygon_2(larger_pts, larger_pts + 4));
	wrapped_nef_polygon smaller(polygon_2(smaller_pts, smaller_pts + 4));
	EXPECT_EQ(
		wrapped_nef_polygon::do_intersect(larger, smaller),
		wrapped_nef_polygon::do_intersect(larger, smaller));
}

TEST(WrappedNefPolygon, BoundingBoxOnAxes) {
	point_2 pts[] = {
		point_2(0, -300),
		point_2(8250, -300),
		point_2(8250, 0),
		point_2(0, 0)
	};
	wrapped_nef_polygon nef(polygon_2(pts, pts + 4));
	bbox_2 bbox = nef.bbox();
	EXPECT_DOUBLE_EQ(0, bbox.xmin());
	EXPECT_DOUBLE_EQ(8250, bbox.xmax());
	EXPECT_DOUBLE_EQ(-300, bbox.ymin());
	EXPECT_DOUBLE_EQ(0, bbox.ymax());
}

TEST(WrappedNefPolygon, BoundingBoxOffAxes) {
	point_2 pts[] = {
		point_2(10, -300),
		point_2(8250, -300),
		point_2(8250, 10),
		point_2(10, 10)
	};
	wrapped_nef_polygon nef(polygon_2(pts, pts + 4));
	bbox_2 bbox = nef.bbox();
	EXPECT_DOUBLE_EQ(10, bbox.xmin());
	EXPECT_DOUBLE_EQ(8250, bbox.xmax());
	EXPECT_DOUBLE_EQ(-300, bbox.ymin());
	EXPECT_DOUBLE_EQ(10, bbox.ymax());
}

TEST(WrappedNefPolygon, BoundingBoxesFilterIntersectionCorrectly) {
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
	wrapped_nef_polygon larger(polygon_2(larger_pts, larger_pts + 4));
	wrapped_nef_polygon smaller(polygon_2(smaller_pts, smaller_pts + 4));
	EXPECT_TRUE(CGAL::do_overlap(larger.bbox(), smaller.bbox()));
}

} // namespace

} // namespace geometry_2d