#include "precompiled.h"

#include <gtest/gtest.h>

#include "geometry_2d_common.h"
#include "sbt-core.h"

extern sb_calculation_options g_opts;

namespace geometry_2d {

TEST(PolygonCleanup, TriplyDuplicatePointAtBeginning) {
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

	bool cleanup_res = cleanup_polygon(&p, g_opts.equality_tolerance);

	EXPECT_TRUE(cleanup_res);
	EXPECT_EQ(correct.container(), p.container());
}

} // namespace geometry_2d