#include "precompiled.h"

#include <gtest/gtest.h>

#include "common.h"
#include "equality_context.h"
#include "oriented_area.h"
#include "simple_face.h"

namespace {

TEST(OrientedArea, SimpleFaceConstruction) {
	equality_context c(0.01);
	face f = create_face(4,
		simple_point(0, 0, 0),
		simple_point(10, 0, 0),
		simple_point(10, 15, 0),
		simple_point(0, 15, 0));
	simple_face(f, &c);
}

TEST(OrientedArea, PositiveZAtZero) {
	equality_context c(0.01);
	oriented_area o(simple_face(create_face(4,
		simple_point(0, 0, 0),
		simple_point(10, 0, 0),
		simple_point(10, 15, 0),
		simple_point(0, 15, 0)), &c), &c);
	EXPECT_EQ(direction_3(0, 0, 1), o.orientation().direction());
	EXPECT_TRUE(o.sense());
	EXPECT_EQ(0, o.height());
}

TEST(OrientedArea, PositiveZAboveZero) {
	equality_context c(0.01);
	oriented_area o(simple_face(create_face(4,
		simple_point(0, 0, 300),
		simple_point(10, 0, 300),
		simple_point(10, 15, 300),
		simple_point(0, 15, 300)), &c), &c);
	EXPECT_EQ(direction_3(0, 0, 1), o.orientation().direction());
	EXPECT_TRUE(o.sense());
	EXPECT_EQ(300, o.height());
}

TEST(OrientedArea, NegativeYAtZero) {
	equality_context c(0.01);
	oriented_area o(simple_face(create_face(4,
		simple_point(0, 0, 0), 
		simple_point(8250, 0, 0),
		simple_point(8250, 0, 300),
		simple_point(0, 0, 300)), &c), &c);
	EXPECT_FALSE(o.sense());
	EXPECT_EQ(0, o.height());
	EXPECT_EQ(direction_3(0, 1, 0), o.orientation().direction());
	EXPECT_EQ(direction_3(0, -1, 0), o.backing_plane().orthogonal_direction());
}

TEST(OrientedArea, ProjectionsIntersection) {
	equality_context c(0.01);

	oriented_area larger(simple_face(create_face(4,
		simple_point(0, 0, 0),
		simple_point(8250, 0, 0),
		simple_point(8250, 0, 300),
		simple_point(0, 0, 300)), &c), &c);

	oriented_area smaller(simple_face(create_face(4,
		simple_point(0, 8250, 300),
		simple_point(4050, 8250, 300),
		simple_point(4050, 8250, 0),
		simple_point(0, 8250, 0)), &c), &c);

	EXPECT_TRUE(area::do_intersect(larger.area_2d(), smaller.area_2d()));
}

TEST(OrientedArea, BackingPlanePointPlacement) {
	equality_context c(0.01);

	oriented_area o(simple_face(create_face(4,
		simple_point(8200, 18195.109, 300),
		simple_point(8200, 17181.249, 300),
		simple_point(8200, 17181.249, 0),
		simple_point(8200, 18195.109, 0)), &c), &c);

	EXPECT_FALSE(o.backing_plane().opposite().has_on_positive_side(point_3(8200, 17181.249, 300)));
	EXPECT_FALSE(o.backing_plane().opposite().has_on_positive_side(point_3(29200, 11911.013, 300)));
	EXPECT_FALSE(o.backing_plane().opposite().has_on_positive_side(point_3(29200, 11911.013, 0)));
	EXPECT_FALSE(o.backing_plane().opposite().has_on_positive_side(point_3(8200, 17181.249, 0)));
}

TEST(OrientedArea, To3d) {
	equality_context c(0.01);

	face f = create_face(4,
		simple_point(8200, 17181.249, 300),
		simple_point(29200, 11911.013, 300),
		simple_point(29200, 11911.013, 0),
		simple_point(8200, 17181.249, 0));

	oriented_area o(simple_face(f, &c), &c);

	polyloop * loop = &f.outer_boundary;

	std::set<point_3> target;
	std::transform(loop->vertices, loop->vertices + 4, std::inserter(target, target.begin()), [&c](point p) {
		return c.request_point(p.x, p.y, p.z);
	});

	auto to3d = o.to_3d();
	ASSERT_EQ(1, to3d.size());
	std::set<point_3> pts;
	boost::transform(to3d.front().outer(), std::inserter(pts, pts.begin()), [&c](const point_3 & p) {
		return c.snap(p);
	});

	EXPECT_EQ(target, pts);
}

TEST(OrientedArea, CouldFormBlock) {
	equality_context c(0.01);

	oriented_area a(simple_face(create_face(4,
		simple_point(4050, 12120.109, 300),
		simple_point(4050, 18195.109, 300),
		simple_point(4050, 18195.109, 0),
		simple_point(4050, 12120.109, 0)), &c), &c);

	oriented_area b(simple_face(create_face(4,
		simple_point(8200, 12120.109, 0),
		simple_point(8200, 18195.109, 0),
		simple_point(8200, 18195.109, 300),
		simple_point(8200, 12120.109, 300)), &c), &c);

	ASSERT_NE(a.sense(), b.sense());
	ASSERT_EQ(&a.orientation(), &b.orientation());
	ASSERT_NE(a.height(), b.height());
	ASSERT_TRUE(a.sense() == a.height() > b.height());
	ASSERT_TRUE(oriented_area::areas_match(a, b));

	EXPECT_TRUE(oriented_area::could_form_block(a, b));
	EXPECT_TRUE(oriented_area::could_form_block(b, a));
}

TEST(OrientedArea, ExplicitCreation) {
	equality_context c(0.01);

	oriented_area oa(simple_face(create_face(4,
		simple_point(4050, 12120, 300),
		simple_point(4050, 18195, 300),
		simple_point(8200, 18195, 300),
		simple_point(8200, 12120, 300)), &c), &c);

	oriented_area copy(&oa.orientation(), oa.height(), oa.area_2d(), oa.sense());

	EXPECT_EQ(&oa.orientation(), &copy.orientation());
	EXPECT_EQ(oa.height(), copy.height());
	EXPECT_EQ(oa.area_2d(), copy.area_2d());
	EXPECT_EQ(oa.sense(), copy.sense());
}

TEST(OrientedArea, MultipleConvexNonAxisFacesToPieces) {
	equality_context c(0.01);

	oriented_area face_1(simple_face(create_face(4,
		simple_point(0, 0, 0),
		simple_point(1, 1, 0),
		simple_point(1, 1, 10),
		simple_point(0, 0, 10)), &c), &c);

	oriented_area face_2(simple_face(create_face(4,
		simple_point(2, 2, 0),
		simple_point(3, 3, 0),
		simple_point(3, 3, 10), 
		simple_point(2, 2, 10)), &c), &c);

	oriented_area both(face_1, face_1.area_2d() + face_2.area_2d());

	std::vector<oriented_area> pieces;
	both.to_pieces(std::back_inserter(pieces));
	
	EXPECT_EQ(2, pieces.size());
}

} // namespace