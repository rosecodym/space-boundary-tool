#include "precompiled.h"

#include <gtest/gtest.h>

#include "area.h"
#include "equality_context.h"
#include "oriented_area.h"
#include "sbt-core-helpers.h"
#include "surface_pair.h"

namespace blocking {

namespace impl {

namespace {

TEST(SurfacePair, StrictSubsetParallelRectangles) {
	equality_context c(0.01);

	face f;
	f.void_count = 0;
	f.voids = nullptr;
	f.outer_boundary.vertex_count = 4;
	f.outer_boundary.vertices = (point *)malloc(sizeof(point) * f.outer_boundary.vertex_count);
	polyloop * loop = get_outer_boundary_handle(&f);

	set_vertex(loop, 0, 0, 0, 0);
	set_vertex(loop, 1, 8250, 0, 0);
	set_vertex(loop, 2, 8250, 0, 300);
	set_vertex(loop, 3, 0, 0, 300);
	oriented_area larger(simple_face(f, &c), &c);

	set_vertex(loop, 0, 0, 8250, 300);
	set_vertex(loop, 1, 4050, 8250, 300);
	set_vertex(loop, 2, 4050, 8250, 0);
	set_vertex(loop, 3, 0, 8250, 0);
	oriented_area smaller(simple_face(f, &c), &c);

	surface_pair pair(larger, smaller, &c);
	EXPECT_TRUE(pair.contributes_to_envelope());
}

TEST(SurfacePair, Nonparallel) {
	equality_context c(0.01);

	face f;
	f.void_count = 0;
	f.voids = nullptr;
	f.outer_boundary.vertex_count = 4;
	f.outer_boundary.vertices = (point *)malloc(sizeof(point) * f.outer_boundary.vertex_count);
	polyloop * loop = get_outer_boundary_handle(&f);

	set_vertex(loop, 0, -8250, 2050, 0);
	set_vertex(loop, 1, -28000, 2050, 0);
	set_vertex(loop, 2, -28000, 2050, 300);
	set_vertex(loop, 3, -8250, 2050, 300);
	oriented_area base(simple_face(f, &c), &c);

	set_vertex(loop, 0, 17181.249, -8200, 300);
	set_vertex(loop, 1, 11991.013, -29200, 300);
	set_vertex(loop, 2, 11991.013, -29200, 0);
	set_vertex(loop, 3, 17181.249, -8200, 0);
	oriented_area other(simple_face(f, &c), &c);

	surface_pair pair(base, other, &c);

	EXPECT_FALSE(pair.is_self());
	EXPECT_FALSE(pair.are_perpendicular());
	EXPECT_FALSE(pair.are_parallel());
	EXPECT_TRUE(pair.other_in_correct_halfspace());
	EXPECT_TRUE(pair.contributes_to_envelope());
}

TEST(SurfacePair, NonparallelAdjoiningIrrelevant) {
	equality_context c(0.01);

	face f;
	f.void_count = 0;
	f.voids = nullptr;
	f.outer_boundary.vertex_count = 4;
	f.outer_boundary.vertices = (point *)malloc(sizeof(point) * f.outer_boundary.vertex_count);
	polyloop * loop = get_outer_boundary_handle(&f);

	set_vertex(loop, 0, 8200, 18195.109, 300);
	set_vertex(loop, 1, 8200, 17181.249, 300);
	set_vertex(loop, 2, 8200, 17181.249, 0);
	set_vertex(loop, 3, 8200, 18195.109, 0);
	oriented_area smaller(simple_face(f, &c), &c);

	set_vertex(loop, 0, 8200, 17181.249, 300);
	set_vertex(loop, 1, 29200, 11911.013, 300);
	set_vertex(loop, 2, 29200, 11911.013, 0);
	set_vertex(loop, 3, 8200, 17181.249, 0);
	oriented_area larger(simple_face(f, &c), &c);

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