#include "precompiled.h"

#include <gtest/gtest.h>

#include "wrapped_nef_polygon.h"

namespace geometry_2d {

namespace {

TEST(WrappedNefPolygonConstruction, OnAxes) {
	point_2 pts[] = {
		point_2(0, -300),
		point_2(8250, -300),
		point_2(8250, 0),
		point_2(0, 0)
	};
	wrapped_nef_polygon nef(polygon_2(pts, pts + 4));
	EXPECT_TRUE(nef.is_axis_aligned());
	bbox_2 bbox = nef.bbox();
	EXPECT_DOUBLE_EQ(0, bbox.xmin());
	EXPECT_DOUBLE_EQ(8250, bbox.xmax());
	EXPECT_DOUBLE_EQ(-300, bbox.ymin());
	EXPECT_DOUBLE_EQ(0, bbox.ymax());
}

TEST(WrappedNefPolygonConstruction, AxisAlignedOffAxes) {
	point_2 pts[] = {
		point_2(10, -300),
		point_2(8250, -300),
		point_2(8250, 10),
		point_2(10, 10)
	};
	wrapped_nef_polygon nef(polygon_2(pts, pts + 4));
	EXPECT_TRUE(nef.is_axis_aligned());
	bbox_2 bbox = nef.bbox();
	EXPECT_DOUBLE_EQ(10, bbox.xmin());
	EXPECT_DOUBLE_EQ(8250, bbox.xmax());
	EXPECT_DOUBLE_EQ(-300, bbox.ymin());
	EXPECT_DOUBLE_EQ(10, bbox.ymax());
}

TEST(WrappedNefPolygonDoIntersect, Subset) {
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
	
	EXPECT_TRUE(wrapped_nef_polygon::do_intersect(larger, smaller));
	// Make sure result doesn't change
	EXPECT_TRUE(wrapped_nef_polygon::do_intersect(larger, smaller)); 
}

TEST(WrappedNefPolygonToSimpleConvexPieces, SingleConvex) {
	point_2 pts[] = {
		point_2(0, 0),
		point_2(393, 0),
		point_2(393, 387),
		point_2(0, 387)
	};
	std::vector<polygon_2> pieces = wrapped_nef_polygon(polygon_2(pts, pts + 4)).to_simple_convex_pieces();
	ASSERT_EQ(1, pieces.size());
	EXPECT_EQ(polygon_2(pts, pts + 4).container(), pieces.front().container());
}

TEST(WrappedNefPolygonFaceCount, SingleFace) {
	point_2 pts[] = {
		point_2(0, 0),
		point_2(393, 0),
		point_2(393, 387),
		point_2(0, 387)
	};
	wrapped_nef_polygon w(polygon_2(pts, pts + 4));
	EXPECT_EQ(1, w.face_count());
}

TEST(WrappedNefPolygonIntersection, Subset) {
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
	wrapped_nef_polygon intr = larger * smaller;
	EXPECT_FALSE(intr.is_empty());
	EXPECT_EQ(1, intr.face_count());
	EXPECT_EQ(4, intr.vertex_count());
}

} // namespace

} // namespace geometry_2d