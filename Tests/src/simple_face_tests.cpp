#include "precompiled.h"

#include <gtest/gtest.h>

#include "common.h"
#include "simple_face.h"

namespace geometry_2d {

namespace {

TEST(SimpleFace, NonPlanarCorrected) {
	equality_context c(0.01);
	simple_face f(create_face(4, 
		simple_point(-6603.14, 675.341, 242.094),
		simple_point(-6603.14, 559.341, 212.989),
		simple_point(-7047.14, 559.341, 212.989),
		simple_point(-7047.14, 795.342, 272.202),
		simple_point(-6603.14, 795.342, 272.202),
		simple_point(-6603.14, 735.342, 257.148),
		simple_point(-6655.14, 735.342, 257.148),
		simple_point(-6655.14, 675.341, 242.094)), &c);
	EXPECT_TRUE(f.is_planar());
}

} // namespace

} // namespace geometry_2d