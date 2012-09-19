#include "precompiled.h"

#include <gtest/gtest.h>

#include "area.h"
#include "cleanup_loop.h"
#include "geometry_common.h"

namespace {

TEST(GeometryCommon, CleanupLoop) {
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
	polygon_2 poly(pts, pts + 7);
	EXPECT_TRUE(geometry_common::cleanup_loop(&poly, 0.01));
	EXPECT_EQ(6, poly.size());
}

TEST(GeometryCommon, InvalidLoop) {
	point_2 pts[] = {
		point_2(27.443970, 8.339190),
		point_2(28.769285, 8.339190),
		point_2(28.769285, 8.844385),
		point_2(28.769105, 8.844385),
		point_2(28.665582, 8.744685),
		point_2(28.665582, 8.844385),
		point_2(27.443970, 8.844385)
	};
	EXPECT_FALSE(area(polygon_2(pts, pts + 7)).is_valid(0.01));
}

}