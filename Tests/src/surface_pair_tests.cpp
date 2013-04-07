#include "precompiled.h"

#include <gtest/gtest.h>

#include "area.h"
#include "common.h"
#include "equality_context.h"
#include "oriented_area.h"
#include "surface_pair.h"

namespace blocking {

namespace impl {

namespace {

TEST(SurfacePair, StrictSubsetParallel) {
	equality_context c(0.01);

	oriented_area larger(simple_face(create_face(4,
		simple_point(0, 0, 0),
		simple_point(8250, 0, 0),
		simple_point(8250, 0, 300),
		simple_point(0, 0, 300)), false, &c), &c);

	oriented_area smaller(simple_face(create_face(4,
		simple_point(0, 8250, 300),
		simple_point(4050, 8250, 300),
		simple_point(4050, 8250, 0),
		simple_point(0, 8250, 0)), false, &c), &c);

	surface_pair pair(larger, smaller, &c);
	EXPECT_TRUE(pair.contributes_to_envelope());
}

TEST(SurfacePair, Nonparallel) {
	equality_context c(0.01);

	oriented_area base(simple_face(create_face(4,
		simple_point(-8250, 2050, 0),
		simple_point(-28000, 2050, 0),
		simple_point(-28000, 2050, 300),
		simple_point(-8250, 2050, 300)), false, &c), &c);

	oriented_area other(simple_face(create_face(4,
		simple_point(17181.249, -8200, 300),
		simple_point(11991.013, -29200, 300),
		simple_point(11991.013, -29200, 0),
		simple_point(17181.249, -8200, 0)), false, &c), &c);

	surface_pair pair(base, other, &c);

	EXPECT_FALSE(pair.is_self());
	EXPECT_FALSE(pair.are_perpendicular());
	EXPECT_FALSE(pair.are_parallel());
	EXPECT_TRUE(pair.other_in_correct_halfspace());
	EXPECT_TRUE(pair.contributes_to_envelope());
}

TEST(SurfacePair, NonparallelAdjoiningIrrelevant) {
	equality_context c(0.01);

	oriented_area smaller(simple_face(create_face(4, 
		simple_point(8200, 18195.109, 300),
		simple_point(8200, 17181.249, 300),
		simple_point(8200, 17181.249, 0),
		simple_point(8200, 18195.109, 0)), false, &c), &c);

	oriented_area larger(simple_face(create_face(4,
		simple_point(8200, 17181.249, 300),
		simple_point(29200, 11911.013, 300),
		simple_point(29200, 11911.013, 0),
		simple_point(8200, 17181.249, 0)), false, &c), &c);

	surface_pair pair(smaller, larger, &c);

	ASSERT_EQ(plane_3(c.request_point(8200, 18195.109, 300), direction_3(1, 0, 0)), smaller.backing_plane()) <<
		"Plane direction is <" << 
		CGAL::to_double(smaller.backing_plane().orthogonal_direction().dx()) << ", " <<
		CGAL::to_double(smaller.backing_plane().orthogonal_direction().dy()) << ", " <<
		CGAL::to_double(smaller.backing_plane().orthogonal_direction().dz()) << ">";

	EXPECT_FALSE(pair.is_self());
	EXPECT_FALSE(pair.are_perpendicular());
	EXPECT_FALSE(pair.are_parallel());
	EXPECT_FALSE(pair.other_in_correct_halfspace());
	EXPECT_TRUE(pair.drape_hits_other_plane());
	EXPECT_FALSE(pair.contributes_to_envelope());
}

} // namespace

} // namespace impl

} // namespace blocking