#include "precompiled.h"

#include <gtest/gtest.h>

#include "common.h"
#include "equality_context.h"
#include "oriented_area.h"
#include "sbt-core.h"
#include "sbt-core-helpers.h"
#include "simple_face.h"

extern sb_calculation_options g_opts;

namespace {

TEST(OrientedArea, SimpleFaceConstruction) {
	equality_context c(g_opts.equality_tolerance);
	face f = create_face(4,
		simple_point(0, 0, 0),
		simple_point(10, 0, 0),
		simple_point(10, 15, 0),
		simple_point(0, 15, 0));
	simple_face(f, &c);
}

TEST(OrientedArea, PositiveZAtZero) {
	equality_context c(g_opts.equality_tolerance);
	face f;
	f.void_count = 0;
	f.voids = nullptr;
	polyloop * loop = get_outer_boundary_handle(&f);
	loop->vertex_count = 4;
	loop->vertices = (point *)malloc(sizeof(point) * loop->vertex_count);
	set_vertex(loop, 0, 0, 0, 0);
	set_vertex(loop, 1, 10, 0, 0);
	set_vertex(loop, 2, 10, 15, 0);
	set_vertex(loop, 3, 0, 15, 0);
	oriented_area o(simple_face(f, &c), &c);
	EXPECT_EQ(direction_3(0, 0, 1), o.orientation().direction());
	EXPECT_TRUE(o.sense());
	EXPECT_EQ(0, o.height());
}

TEST(OrientedArea, PositiveZAboveZero) {
	equality_context c(g_opts.equality_tolerance);
	face f;
	f.void_count = 0;
	f.voids = nullptr;
	polyloop * loop = get_outer_boundary_handle(&f);
	loop->vertex_count = 4;
	loop->vertices = (point *)malloc(sizeof(point) * loop->vertex_count);
	set_vertex(loop, 0, 0, 0, 300);
	set_vertex(loop, 1, 10, 0, 300);
	set_vertex(loop, 2, 10, 15, 300);
	set_vertex(loop, 3, 0, 15, 300);
	oriented_area o(simple_face(f, &c), &c);
	EXPECT_EQ(direction_3(0, 0, 1), o.orientation().direction());
	EXPECT_TRUE(o.sense());
	EXPECT_EQ(300, o.height());
}

TEST(OrientedArea, NegativeYAtZero) {
	equality_context c(g_opts.equality_tolerance);
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
	oriented_area o(simple_face(f, &c), &c);
	EXPECT_FALSE(o.sense());
	EXPECT_EQ(0, o.height());
	EXPECT_EQ(direction_3(0, 1, 0), o.orientation().direction());
	EXPECT_EQ(direction_3(0, -1, 0), o.backing_plane().orthogonal_direction());
}

TEST(OrientedArea, ProjectionsIntersection) {
	equality_context c(g_opts.equality_tolerance);

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

	EXPECT_TRUE(area::do_intersect(larger.area_2d(), smaller.area_2d()));
}

TEST(OrientedArea, BackingPlanePointPlacement) {
	equality_context c(g_opts.equality_tolerance);

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
	oriented_area o(simple_face(f, &c), &c);

	EXPECT_FALSE(o.backing_plane().opposite().has_on_positive_side(point_3(8200, 17181.249, 300)));
	EXPECT_FALSE(o.backing_plane().opposite().has_on_positive_side(point_3(29200, 11911.013, 300)));
	EXPECT_FALSE(o.backing_plane().opposite().has_on_positive_side(point_3(29200, 11911.013, 0)));
	EXPECT_FALSE(o.backing_plane().opposite().has_on_positive_side(point_3(8200, 17181.249, 0)));
}

TEST(OrientedArea, To3d) {
	equality_context c(g_opts.equality_tolerance);

	face f;
	f.void_count = 0;
	f.voids = nullptr;
	f.outer_boundary.vertex_count = 4;
	f.outer_boundary.vertices = (point *)malloc(sizeof(point) * f.outer_boundary.vertex_count);
	polyloop * loop = get_outer_boundary_handle(&f);

	set_vertex(loop, 0, 8200, 17181.249, 300);
	set_vertex(loop, 1, 29200, 11911.013, 300);
	set_vertex(loop, 2, 29200, 11911.013, 0);
	set_vertex(loop, 3, 8200, 17181.249, 0);
	oriented_area o(simple_face(f, &c), &c);

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
	equality_context c(g_opts.equality_tolerance);

	face f;
	f.void_count = 0;
	f.voids = nullptr;
	f.outer_boundary.vertex_count = 4;
	f.outer_boundary.vertices = (point *)malloc(sizeof(point) * f.outer_boundary.vertex_count);
	polyloop * loop = get_outer_boundary_handle(&f);

	set_vertex(loop, 0, 4050, 12120.109, 300);
	set_vertex(loop, 1, 4050, 18195.109, 300);
	set_vertex(loop, 2, 4050, 18195.109, 0);
	set_vertex(loop, 3, 4050, 12120.109, 0);
	oriented_area a(simple_face(f, &c), &c);

	set_vertex(loop, 0, 8200, 12120.109, 0);
	set_vertex(loop, 1, 8200, 18195.109, 0);
	set_vertex(loop, 2, 8200, 18195.109, 300);
	set_vertex(loop, 3, 8200, 12120.109, 300);
	oriented_area b(simple_face(f, &c), &c);

	ASSERT_NE(a.sense(), b.sense());
	ASSERT_EQ(&a.orientation(), &b.orientation());
	ASSERT_NE(a.height(), b.height());
	ASSERT_TRUE(a.sense() == a.height() > b.height());
	ASSERT_TRUE(oriented_area::areas_match(a, b));

	EXPECT_TRUE(oriented_area::could_form_block(a, b));
	EXPECT_TRUE(oriented_area::could_form_block(b, a));
}

} // namespace