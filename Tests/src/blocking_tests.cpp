#include "precompiled.h"

#include <gtest/gtest.h>

#include "build_blocks.h"
#include "element.h"
#include "halfblocks_for_base.h"
#include "link_halfblocks.h"
#include "oriented_area.h"
#include "sbt-core.h"
#include "sbt-core-helpers.h"
#include "simple_face.h"
#include "surface_pair.h"

extern sb_calculation_options g_opts;

namespace blocking {

namespace impl {

namespace {

TEST(Blocking, ParallelPairHalfblocks) {
	equality_context c(g_opts.equality_tolerance);

	std::vector<oriented_area> surfaces;

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
	surfaces.push_back(oriented_area(simple_face(f, &c), &c));
	ASSERT_FALSE(surfaces.back().sense()) << "(Near)";
	ASSERT_EQ(0, surfaces.back().height()) << "(Near)";
	ASSERT_EQ(direction_3(0, 1, 0), surfaces.back().orientation().direction()) << "(Near)";

	set_vertex(loop, 0, 0, 8250, 300);
	set_vertex(loop, 1, 8250, 8250, 300);
	set_vertex(loop, 2, 8250, 8250, 0);
	set_vertex(loop, 3, 0, 8250, 0);
	surfaces.push_back(oriented_area(simple_face(f, &c), &c));
	ASSERT_TRUE(surfaces.back().sense()) << "(Far)";
	ASSERT_EQ(8250, surfaces.back().height()) << "(Far)";
	ASSERT_EQ(direction_3(0, 1, 0), surfaces.back().orientation().direction()) << "(Far)";

	std::vector<oriented_area> halfblocks;

	auto rels = build_relations_grid(surfaces, &c);

	halfblocks.clear();
	halfblocks_for_base(rels, 0, &c, std::back_inserter(halfblocks));
	EXPECT_EQ(1, halfblocks.size()) << "(Near)";

	halfblocks.clear();
	halfblocks_for_base(rels, 1, &c, std::back_inserter(halfblocks));
	EXPECT_EQ(1, halfblocks.size()) << "(Far)";
}

TEST(Blocking, TwoStairs) {
	equality_context c(g_opts.equality_tolerance);

	std::vector<oriented_area> surfaces;

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
	surfaces.push_back(oriented_area(simple_face(f, &c), &c));
	ASSERT_FALSE(surfaces.back().sense()) << "(Base)";
	ASSERT_EQ(0, surfaces.back().height()) << "(Base)";
	ASSERT_EQ(direction_3(0, 1, 0), surfaces.back().orientation().direction()) << "(Base)";

	set_vertex(loop, 0, 0, 8250, 300);
	set_vertex(loop, 1, 4050, 8250, 300);
	set_vertex(loop, 2, 4050, 8250, 0);
	set_vertex(loop, 3, 0, 8250, 0);
	surfaces.push_back(oriented_area(simple_face(f, &c), &c));
	ASSERT_TRUE(surfaces.back().sense()) << "(Near)";
	ASSERT_EQ(8250, surfaces.back().height()) << "(Near)";
	ASSERT_EQ(direction_3(0, 1, 0), surfaces.back().orientation().direction()) << "(Near)";

	set_vertex(loop, 0, 4050, 18195.109, 300);
	set_vertex(loop, 1, 8250, 18195.109, 300);
	set_vertex(loop, 2, 8250, 18195.109, 0);
	set_vertex(loop, 3, 4050, 18195.109, 0);
	surfaces.push_back(oriented_area(simple_face(f, &c), &c));
	ASSERT_TRUE(surfaces.back().sense()) << "(Far)";
	ASSERT_EQ(18195.109, surfaces.back().height()) << "(Far)";
	ASSERT_EQ(direction_3(0, 1, 0), surfaces.back().orientation().direction()) << "(Far)";

	relations_grid rels = build_relations_grid(surfaces, &c);

	const surface_pair & bb = rels[0][0];
	EXPECT_TRUE(bb.is_self());
	EXPECT_FALSE(bb.contributes_to_envelope());

	const surface_pair & bn = rels[0][1];
	EXPECT_FALSE(bn.is_self());
	EXPECT_FALSE(bn.are_perpendicular());
	EXPECT_TRUE(bn.are_parallel());
	EXPECT_TRUE(bn.base().sense() == !bn.other().sense());
	EXPECT_TRUE(bn.base().height() > bn.other().height() == bn.base().sense());
	EXPECT_TRUE(area::do_intersect(bn.base().area_2d(), bn.other().area_2d()));
	EXPECT_TRUE(bn.contributes_to_envelope());

	const surface_pair & bf = rels[0][2];
	EXPECT_FALSE(bf.is_self());
	EXPECT_FALSE(bf.are_perpendicular());
	EXPECT_TRUE(bf.are_parallel());
	EXPECT_TRUE(bf.base().sense() == !bf.other().sense());
	EXPECT_TRUE(bf.base().height() > bf.other().height() == bf.base().sense());
	EXPECT_TRUE(area::do_intersect(bf.base().area_2d(), bf.other().area_2d()));
	EXPECT_TRUE(bf.contributes_to_envelope());

	const surface_pair & nb = rels[1][0];
	EXPECT_FALSE(nb.is_self());
	EXPECT_FALSE(nb.are_perpendicular());
	EXPECT_TRUE(nb.are_parallel());
	EXPECT_TRUE(nb.base().sense() == !nb.other().sense());
	EXPECT_TRUE(nb.base().height() > nb.other().height() == nb.base().sense());
	EXPECT_TRUE(area::do_intersect(nb.base().area_2d(), nb.other().area_2d())) <<
		(nb.base().area_2d().print(), "") << (nb.other().area_2d().print(), "");
	EXPECT_TRUE(nb.contributes_to_envelope());

	const surface_pair & nn = rels[1][1];
	EXPECT_TRUE(nn.is_self());
	EXPECT_FALSE(nn.contributes_to_envelope());

	const surface_pair & nf = rels[1][2];
	EXPECT_FALSE(nf.is_self());
	EXPECT_FALSE(nf.are_perpendicular());
	EXPECT_TRUE(nf.are_parallel());
	EXPECT_FALSE(nf.base().sense() == !nf.other().sense());
	EXPECT_FALSE(nf.contributes_to_envelope());

	const surface_pair & fb = rels[2][0];
	EXPECT_FALSE(fb.is_self());
	EXPECT_FALSE(fb.are_perpendicular());
	EXPECT_TRUE(fb.are_parallel());
	EXPECT_TRUE(fb.base().sense() == !fb.other().sense());
	EXPECT_TRUE(fb.base().height() > fb.other().height() == fb.base().sense());
	EXPECT_TRUE(area::do_intersect(fb.base().area_2d(), fb.other().area_2d()));
	EXPECT_TRUE(fb.contributes_to_envelope());

	const surface_pair & fn = rels[2][1];
	EXPECT_FALSE(fn.is_self());
	EXPECT_FALSE(fn.are_perpendicular());
	EXPECT_TRUE(fn.are_parallel());
	EXPECT_FALSE(fn.base().sense() == !fn.other().sense());
	EXPECT_FALSE(fn.contributes_to_envelope());

	const surface_pair & ff = rels[2][2];
	EXPECT_TRUE(ff.is_self());
	EXPECT_FALSE(ff.contributes_to_envelope());

	std::vector<oriented_area> halfblocks;

	halfblocks_for_base(rels, 0, &c, std::back_inserter(halfblocks));
	EXPECT_EQ(2, halfblocks.size()) << "(Base)";

	halfblocks.clear();
	halfblocks_for_base(rels, 1, &c, std::back_inserter(halfblocks));
	EXPECT_EQ(1, halfblocks.size()) << "(Near)";

	halfblocks.clear();
	halfblocks_for_base(rels, 2, &c, std::back_inserter(halfblocks));
	EXPECT_EQ(1, halfblocks.size()) << "(Far)";
}

TEST(Blocking, ThreeStairsHalfblocks) {
	equality_context c(g_opts.equality_tolerance);

	std::vector<oriented_area> surfaces;

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
	surfaces.push_back(oriented_area(simple_face(f, &c), &c));
	ASSERT_FALSE(surfaces.back().sense()) << "(Base)";
	ASSERT_EQ(0, surfaces.back().height()) << "(Base)";
	ASSERT_EQ(direction_3(0, 1, 0), surfaces.back().orientation().direction()) << "(Base)";

	set_vertex(loop, 0, 0, 8250, 300);
	set_vertex(loop, 1, 2105, 8250, 300);
	set_vertex(loop, 2, 2105, 8250, 0);
	set_vertex(loop, 3, 0, 8250, 0);
	surfaces.push_back(oriented_area(simple_face(f, &c), &c));
	ASSERT_TRUE(surfaces.back().sense()) << "(Near)";
	ASSERT_EQ(8250, surfaces.back().height()) << "(Near)";
	ASSERT_EQ(direction_3(0, 1, 0), surfaces.back().orientation().direction()) << "(Near)";

	set_vertex(loop, 0, 2105, 12120.109, 300);
	set_vertex(loop, 1, 4050, 12120.109, 300);
	set_vertex(loop, 2, 4050, 12120.109, 0);
	set_vertex(loop, 3, 2105, 12120.109, 0);
	surfaces.push_back(oriented_area(simple_face(f, &c), &c));
	ASSERT_TRUE(surfaces.back().sense()) << "(Mid)";
	ASSERT_EQ(12120.109, surfaces.back().height()) << "(Mid)";
	ASSERT_EQ(direction_3(0, 1, 0), surfaces.back().orientation().direction()) << "(Mid)";

	set_vertex(loop, 0, 4050, 18195.109, 300);
	set_vertex(loop, 1, 8250, 18195.109, 300);
	set_vertex(loop, 2, 8250, 18195.109, 0);
	set_vertex(loop, 3, 4050, 18195.109, 0);
	surfaces.push_back(oriented_area(simple_face(f, &c), &c));
	ASSERT_TRUE(surfaces.back().sense()) << "(Far)";
	ASSERT_EQ(18195.109, surfaces.back().height()) << "(Far)";
	ASSERT_EQ(direction_3(0, 1, 0), surfaces.back().orientation().direction()) << "(Far)";

	relations_grid rels = build_relations_grid(surfaces, &c);
	std::vector<oriented_area> halfblocks;

	halfblocks_for_base(rels, 0, &c, std::back_inserter(halfblocks));
	EXPECT_EQ(3, halfblocks.size()) << "(Base)";

	halfblocks.clear();
	halfblocks_for_base(rels, 1, &c, std::back_inserter(halfblocks));
	EXPECT_EQ(1, halfblocks.size()) << "(Near)";

	halfblocks.clear();
	halfblocks_for_base(rels, 2, &c, std::back_inserter(halfblocks));
	EXPECT_EQ(1, halfblocks.size()) << "(Mid)";

	halfblocks.clear();
	halfblocks_for_base(rels, 3, &c, std::back_inserter(halfblocks));
	EXPECT_EQ(1, halfblocks.size()) << "(Far)";
}

TEST(Blocking, ThreeStairsElement) { 
	equality_context c(g_opts.equality_tolerance);

	element_info e_info;
	solid * geom = get_element_geometry_handle(&e_info);
	set_to_extruded_area_solid(geom, 0, 0, 1, 300);
	face * f = get_area_handle(geom);
	polyloop * loop = get_outer_boundary_handle(f);

	set_vertex_count(loop, 8);
	set_vertex(loop, 0, 0, 0, 0);
	set_vertex(loop, 1, 0, 8250, 0);
	set_vertex(loop, 2, 2105, 8250, 0);
	set_vertex(loop, 3, 2105, 12120.109, 0);
	set_vertex(loop, 4, 4050, 12120.109, 0);
	set_vertex(loop, 5, 4050, 18195.109, 0);
	set_vertex(loop, 6, 8200, 18195.109, 0);
	set_vertex(loop, 7, 8200, 0, 0);

	element e(&e_info, &c);
	auto blocks = build_blocks_for(e, &c);
	ASSERT_EQ(10, e.faces(&c).size());
	EXPECT_EQ(7, blocks.size());
}

TEST(Blocking, NonparallelPairHalfblocksForSmallerBase) {
	equality_context c(g_opts.equality_tolerance);

	face f;
	f.void_count = 0;
	f.voids = nullptr;
	f.outer_boundary.vertex_count = 4;
	f.outer_boundary.vertices = (point *)malloc(sizeof(point) * f.outer_boundary.vertex_count);
	polyloop * loop = get_outer_boundary_handle(&f);

	std::vector<oriented_area> surfaces;

	set_vertex(loop, 0, -8250, 2050, 0);
	set_vertex(loop, 1, -28000, 2050, 0);
	set_vertex(loop, 2, -28000, 2050, 300);
	set_vertex(loop, 3, -8250, 2050, 300);
	surfaces.push_back(oriented_area(simple_face(f, &c), &c));

	set_vertex(loop, 0, 17181.249, -8200, 300);
	set_vertex(loop, 1, 11991.013, -29200, 300);
	set_vertex(loop, 2, 11991.013, -29200, 0);
	set_vertex(loop, 3, 17181.249, -8200, 0);
	surfaces.push_back(oriented_area(simple_face(f, &c), &c));

	auto rels = build_relations_grid(surfaces, &c);

	std::vector<oriented_area> halfblocks;
	halfblocks_for_base(rels, 0, &c, std::back_inserter(halfblocks));
	EXPECT_EQ(1, halfblocks.size());
}

TEST(Blocking, HalfblocksWithNonParallelOthers) {
	equality_context c(g_opts.equality_tolerance);

	face f;
	f.void_count = 0;
	f.voids = nullptr;
	f.outer_boundary.vertex_count = 4;
	f.outer_boundary.vertices = (point *)malloc(sizeof(point) * f.outer_boundary.vertex_count);
	polyloop * loop = get_outer_boundary_handle(&f);
	
	std::vector<oriented_area> surfaces;

	set_vertex(loop, 0, 4050, 12120.109, 300);
	set_vertex(loop, 1, 4050, 18195.109, 300);
	set_vertex(loop, 2, 4050, 18195.109, 0);
	set_vertex(loop, 2, 4050, 12120.109, 0);
	surfaces.push_back(oriented_area(simple_face(f, &c), &c));

	ASSERT_EQ(4, surfaces.back().area_2d().outer_bound().size());

	set_vertex(loop, 0, 8200, 17181.249, 0);
	set_vertex(loop, 1, 8200, 18195.109, 0);
	set_vertex(loop, 2, 8200, 18195.109, 300);
	set_vertex(loop, 3, 8200, 17181.249, 300);
	surfaces.push_back(oriented_area(simple_face(f, &c), &c));

	ASSERT_EQ(4, surfaces.back().area_2d().outer_bound().size());

	set_vertex(loop, 0, 8200, 17181.249, 300);
	set_vertex(loop, 1, 29200, 11991.913, 300);
	set_vertex(loop, 2, 29200, 11991.913, 0);
	set_vertex(loop, 3, 8200, 17181.249, 0);
	surfaces.push_back(oriented_area(simple_face(f, &c), &c));

	ASSERT_EQ(4, surfaces.back().area_2d().outer_bound().size());

	auto rels = build_relations_grid(surfaces, &c);

	std::vector<oriented_area> halfblocks;
	halfblocks_for_base(rels, 0, &c, std::back_inserter(halfblocks));
	EXPECT_EQ(2, halfblocks.size());
}

TEST(Blocking, Boot) {
	equality_context c(g_opts.equality_tolerance);

	element_info e_info;
	strcpy(e_info.id, "boot element");
	solid * geom = get_element_geometry_handle(&e_info);
	set_to_extruded_area_solid(geom, 0, 0, 1, 300);
	face * f = get_area_handle(geom);
	polyloop * loop = get_outer_boundary_handle(f);

	set_vertex_count(loop, 5);
	set_vertex(loop, 0, 4050, 12120.109, 0);
	set_vertex(loop, 1, 4050, 18195.109, 0);
	set_vertex(loop, 2, 8200, 18195.109, 0);
	set_vertex(loop, 3, 8200, 17181.249, 0);
	set_vertex(loop, 4, 29200, 11991.013, 0);
	
	element e(&e_info, &c);

	auto blocks = build_blocks_for(e, &c);
	EXPECT_EQ(6, blocks.size());
}

TEST(Blocking, SinglePairLink) {
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

	ASSERT_TRUE(oriented_area::could_form_block(a, b));

	std::list<oriented_area> surfaces;
	surfaces.push_back(a);
	surfaces.push_back(b);

	element_info dummy_element;
	strcpy(dummy_element.id, "dummy element");
	set_to_extruded_area_solid(&dummy_element.geometry, 0.0, 0.0, 1.0, 100);
	loop = get_outer_boundary_handle(get_area_handle(&dummy_element.geometry));
	set_vertex_count(loop, 4);
	set_vertex(loop, 0, 0, 0, 0);
	set_vertex(loop, 1, 1, 0, 0);
	set_vertex(loop, 2, 1, 1, 0);
	set_vertex(loop, 3, 0, 1, 0);
	element dummy(&dummy_element, &c);

	std::vector<block> blocks;
	link_halfblocks(std::move(surfaces), dummy, std::back_inserter(blocks));
	EXPECT_EQ(1, blocks.size());
}

} // namespace

} // namespace impl

} // namespace blocking