#include "precompiled.h"

#include <gtest/gtest.h>

#include "common.h"
#include "simple_face.h"

namespace geometry_2d {

namespace {

TEST(SimpleFaceXYTriangle, HasCorrectVertices) {
	equality_context c(0.01);
	simple_face f(create_face(3,
		simple_point(0, 0, 0),
		simple_point(1, 0, 0),
		simple_point(0, 1, 0)), &c);

	EXPECT_EQ(3, f.outer().size());
	EXPECT_EQ(0, f.inners().size());

	auto has_point = [&f](double x, double y, double z) {
		return 
			boost::find(f.outer(), point_3(x, y, z)) !=
			f.outer().end();
	};

	EXPECT_TRUE(has_point(0, 0, 0));
	EXPECT_TRUE(has_point(1, 0, 0));
	EXPECT_TRUE(has_point(0, 1, 0));
}

TEST(SimpleFaceFlatRectangle, HasCorrectAveragePoint) {
	equality_context c(0.01);
	simple_face f(create_face(4,
		simple_point(0, 0, 300),
		simple_point(10, 0, 300),
		simple_point(10, 15, 300),
		simple_point(0, 15, 300)), &c);

	EXPECT_EQ(point_3(5, 7.5, 300), f.average_outer_point());
}

TEST(SimpleFaceNonAxisTriangle, HasCorrectVertices) {
	equality_context c(0.01);
	simple_face f(create_face(3,
		simple_point(1, 0, 0),
		simple_point(0, 1, 0),
		simple_point(0, 0, 1)), &c);

	EXPECT_EQ(3, f.outer().size());
	EXPECT_EQ(0, f.inners().size());

	auto has_point = [&f](double x, double y, double z) {
		return 
			boost::find(f.outer(), point_3(x, y, z)) !=
			f.outer().end();
	};

	EXPECT_TRUE(has_point(1, 0, 0));
	EXPECT_TRUE(has_point(0, 1, 0));
	EXPECT_TRUE(has_point(0, 0, 1));
}

TEST(SimpleFaceNonPlanar, EndsUpPlanar) {
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