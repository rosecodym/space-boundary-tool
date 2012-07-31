#include "precompiled.h"

#include <gtest/gtest.h>

#include "geometry_common.h"
#include "sbt-core.h"

extern sb_calculation_options g_opts;

namespace geometry_2d {

TEST(LoopCleanup, TriplyDuplicatePointAtBeginningPolygon2) {
	point_2 pts[] = {
		point_2(4030.886788, 18109.240611),
		point_2(4030.886788, 18109.240611),
		point_2(4030.886788, 18109.240611),
		point_2(3921.665085, 17618.549059),
		point_2(4214.498547, 17553.368053),
		point_2(4323.720250, 18044.059604)
	};
	polygon_2 p(pts, pts + 6);
	polygon_2 correct(pts + 2, pts + 6);

	bool cleanup_res = geometry_common::cleanup_loop(&p, g_opts.equality_tolerance);

	EXPECT_TRUE(cleanup_res);
	EXPECT_EQ(correct.container(), p.container());
}

TEST(LoopCleanup, ThreePointDuplicateBeginningPoint3) {
	point_3 pts[] = {
		point_3(3.723846, 11.736685, 9.385000),
		point_3(3.723846, 11.736685, 9.385000),
		point_3(3.723846, 11.758527, 9.385000)
	};
	std::vector<point_3> vec(pts, pts + 3);
	EXPECT_FALSE(geometry_common::cleanup_loop(&vec, g_opts.equality_tolerance));
}

TEST(LoopCleanup, ThreePointDuplicateEndPoint3) {
	point_3 pts[] = {
		point_3(3.723846, 13.682886, 9.385000),
		point_3(3.736578, 13.695610, 9.385000),
		point_3(3.736578, 13.695610, 9.385000)
	};
	std::vector<point_3> vec(pts, pts + 3);
	EXPECT_FALSE(geometry_common::cleanup_loop(&vec, g_opts.equality_tolerance));
}

TEST(LoopCleanup, DirectionChangePoint2) {
	point_2 pts[] = {
		point_2(-9.0, 205.650477),
		point_2(-9.0, 205.983810),
		point_2(-9.0, 204.067143),
		point_2(0.0, 204.067143),
		point_2(0.0, 205.650477)
	};
	std::vector<point_2> vec(pts, pts + 5);
	ASSERT_TRUE(geometry_common::cleanup_loop(&vec, g_opts.equality_tolerance));
	EXPECT_EQ(4, vec.size());
}

// strictly speaking, this test is capturing a bug
// (specifically, "cleaned" loops coming out of cleanup_loop aren't necessarily clean!)
TEST(LoopCleanup, ComplexTwoPassesPoint2) {
	point_2 pts[] = {
		point_2(-9.000000, 205.650477),
		point_2(-9.000000, 205.983810),
		point_2(0.000000, 205.983810),
		point_2(0.000000, 205.983810),
		point_2(-9.000000, 205.983810),
		point_2(-9.000000, 204.067143),
		point_2(0.000000, 204.067143),
		point_2(0.000000, 205.650477)
	};
	std::vector<point_2> vec(pts, pts + 8);
	ASSERT_TRUE(geometry_common::cleanup_loop(&vec, g_opts.equality_tolerance));
	ASSERT_EQ(5, vec.size());
	ASSERT_TRUE(geometry_common::cleanup_loop(&vec, g_opts.equality_tolerance));
	EXPECT_EQ(4, vec.size());
}

} // namespace geometry_2d