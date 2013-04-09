#include "precompiled.h"

#include <gtest/gtest.h>

#include "geometry_common.h"

#include "common.h"
#include "equality_context.h"

namespace geometry_common {

namespace {

TEST(CalculatePlaneAndAveragePointNonPlanar, ClampsCorrectly) {
	equality_context c(0.01);
	std::vector<point_3> pts;
	auto add = [&c, &pts](double x, double y, double z) {
		pts.push_back(c.request_point(x, y, z));
	};
	// This is an actual poly in one of our test models that needs to work.
	// (Specifically, it's the base of the slab 2Xdgnm1yT5kByBceZDxlSy in the
	// ArchiCAD 11 USCG Pubworks building, *after* the equality context has
	// pulled some coordinates over from the base of column
	// 11_jh73U5AEAyqTZSg0a1r. See issue 152.)
	add(-6603.135668, 675.341498, 242.093835);
	add(-6603.135668, 559.341436, 212.989174);
	add(-7047.135666, 559.341436, 212.989174);
	add(-7047.135666, 795.341440, 272.202105);
	add(-6603.135668, 795.341440, 272.202105);
	add(-6603.135668, 735.341523, 257.147969);
	add(-6655.135668, 735.341523, 257.147969);
	add(-6655.135668, 675.341498, 242.093835);
	plane_3 pl = std::get<0>(calculate_plane_and_average_point(pts, c));
	EXPECT_EQ(0, pl.orthogonal_direction().dx());
}

} // namespace

} // namespace geometry_commo