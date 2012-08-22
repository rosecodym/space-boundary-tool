#include "precompiled.h"

#include <gtest/gtest.h>

#include "build_blocks.h"
#include "build_stacks.h"
#include "common.h"
#include "sbt-core.h"
#include "space.h"

extern sb_calculation_options g_opts;

namespace stacking {

namespace impl {

namespace {

TEST(Stacking, DoConnect) {
	equality_context c(g_opts.equality_tolerance);

	face f;
	f = create_face(4,
		simple_point(0, 0, 0),
		simple_point(393, 0, 0),
		simple_point(393, 387, 0),
		simple_point(0, 387, 0));

	space_info * s_info = create_space("space", create_ext(0, 0, 1, 300, f));
	element_info * e_info = create_element("element", SLAB, 1, create_ext(0, 0, 1, 300, f));

	space dummy_space(s_info, &c);
	element dummy_element(e_info, &c);
	
	oriented_area lower_up(simple_face(f, &c), &c);
	oriented_area lower_down = lower_up.reverse();
	
	f.outer_boundary.vertices[0].z = 300;
	f.outer_boundary.vertices[1].z = 300;
	f.outer_boundary.vertices[2].z = 300;
	f.outer_boundary.vertices[3].z = 300;

	oriented_area upper_up(simple_face(f, &c), &c);
	oriented_area upper_down = upper_up.reverse();

	space_face bottom_lower_face(&dummy_space, lower_down);
	space_face bottom_upper_face(&dummy_space, upper_up);
	space_face top_lower_face(&dummy_space, upper_down);

	block bottom_block(lower_down, upper_up, dummy_element);
	block top_block(upper_down, dummy_element);

	stackable bottom_lower_face_s(&bottom_lower_face);
	stackable bottom_upper_face_s(&bottom_upper_face);
	stackable top_lower_face_s(&top_lower_face);
	stackable bottom_block_s(&bottom_block);
	stackable top_block_s(&top_block);

	boost::optional<stackable_connection> cnct;

	double eps = g_opts.equality_tolerance;
	EXPECT_FALSE(stackable_connection::do_connect(bottom_lower_face_s, bottom_lower_face_s, eps));
	EXPECT_FALSE(stackable_connection::do_connect(bottom_lower_face_s, bottom_upper_face_s, eps));
	EXPECT_FALSE(stackable_connection::do_connect(bottom_lower_face_s, top_lower_face_s, eps));
	EXPECT_FALSE(stackable_connection::do_connect(bottom_lower_face_s, bottom_block_s, eps));
	EXPECT_FALSE(stackable_connection::do_connect(bottom_lower_face_s, top_block_s, eps));
	EXPECT_FALSE(stackable_connection::do_connect(bottom_upper_face_s, bottom_lower_face_s, eps));
	EXPECT_FALSE(stackable_connection::do_connect(bottom_upper_face_s, bottom_upper_face_s, eps));
	EXPECT_TRUE(cnct = stackable_connection::do_connect(bottom_upper_face_s, top_lower_face_s, eps));
	if (cnct) { EXPECT_DOUBLE_EQ(300, cnct->connection_height) << "lower space face-upper space face"; }
	EXPECT_FALSE(stackable_connection::do_connect(bottom_upper_face_s, bottom_block_s, eps));
	EXPECT_TRUE(cnct = stackable_connection::do_connect(bottom_upper_face_s, top_block_s, eps));
	if (cnct) { EXPECT_DOUBLE_EQ(300, cnct->connection_height) << "lower space face-upper block"; }
	EXPECT_FALSE(stackable_connection::do_connect(top_lower_face_s, bottom_lower_face_s, eps));
	EXPECT_TRUE(stackable_connection::do_connect(top_lower_face_s, bottom_upper_face_s, eps));
	if (cnct) { EXPECT_DOUBLE_EQ(300, cnct->connection_height) << "upper space face-lower space face"; }
	EXPECT_FALSE(stackable_connection::do_connect(top_lower_face_s, top_lower_face_s, eps));
	EXPECT_TRUE(stackable_connection::do_connect(top_lower_face_s, bottom_block_s, eps));
	if (cnct) { EXPECT_DOUBLE_EQ(300, cnct->connection_height) << "upper space face-lower block"; }
	EXPECT_FALSE(stackable_connection::do_connect(top_lower_face_s, top_block_s, eps));
	EXPECT_FALSE(stackable_connection::do_connect(bottom_block_s, bottom_lower_face_s, eps));
	EXPECT_FALSE(stackable_connection::do_connect(bottom_block_s, bottom_upper_face_s, eps));
	EXPECT_TRUE(cnct = stackable_connection::do_connect(bottom_block_s, top_lower_face_s, eps));
	if (cnct) { EXPECT_DOUBLE_EQ(300, cnct->connection_height) << "lower block-upper space face"; }
	EXPECT_FALSE(stackable_connection::do_connect(bottom_block_s, bottom_block_s, eps));
	EXPECT_TRUE(stackable_connection::do_connect(bottom_block_s, top_block_s, eps));
	if (cnct) { EXPECT_DOUBLE_EQ(300, cnct->connection_height) << "lower block-upper block"; }
	EXPECT_FALSE(stackable_connection::do_connect(top_block_s, bottom_lower_face_s, eps));
	EXPECT_TRUE(cnct = stackable_connection::do_connect(top_block_s, bottom_upper_face_s, eps));
	if (cnct) { EXPECT_DOUBLE_EQ(300, cnct->connection_height) << "upper block-lower space face"; }
	EXPECT_FALSE(stackable_connection::do_connect(top_block_s, top_lower_face_s, eps)); 
	EXPECT_TRUE(cnct = stackable_connection::do_connect(top_block_s, bottom_block_s, eps));
	if (cnct) { EXPECT_DOUBLE_EQ(300, cnct->connection_height); }
	EXPECT_FALSE(stackable_connection::do_connect(top_block_s, top_block_s, eps));
}

TEST(Stacking, SingleSpaceFaces) {
	equality_context c(g_opts.equality_tolerance);

	space_info * s_info = create_space("space", create_ext(0, 0, 1, 307.08661, create_face(5,
		simple_point(0, 0, 0),
		simple_point(393.70079, 0, 0),
		simple_point(393.70079, 387.79528, 0),
		simple_point(0, 387.79528, 0),
		simple_point(0, 0, 0))));

	std::vector<space> spaces(1, space(s_info, &c));

	auto faces = get_space_faces_by_orientation(spaces, &c);

	EXPECT_EQ(3, faces.size());
}

TEST(Stacking, IsolatedSpace) {
	equality_context c(g_opts.equality_tolerance);

	space_info * s_info = create_space("space", create_ext(0, 0, 1, 307.08661, create_face(5,
		simple_point(0, 0, 0),
		simple_point(393.70079, 0, 0),
		simple_point(393.70079, 387.79528, 0),
		simple_point(0, 387.79528, 0),
		simple_point(0, 0, 0))));

	std::vector<space> spaces(1, space(s_info, &c));
	std::vector<block> blocks;

	EXPECT_EQ(0, stacking::build_stacks(blocks, spaces, g_opts.equality_tolerance, &c).size());
}

TEST(Stacking, FloorAndRoomStackingGraph) {
	equality_context c(g_opts.equality_tolerance);

	space_info * s_info = create_space("space", create_ext(0, 0, 1, 307.08661, create_face(5,
		simple_point(0, 0, 0),
		simple_point(393.70079, 0, 0),
		simple_point(393.70079, 387.79528, 0),
		simple_point(0, 387.79528, 0),
		simple_point(0, 0, 0))));

	element_info * e_info = create_element("floor", SLAB, 1, create_ext(0, 0, 1, 7.8740157, create_face(5,
		simple_point(0, 0, -7.8740157),
		simple_point(0, 409.44882, -7.8740157),
		simple_point(803.14961, 409.44882, -7.8740157),
		simple_point(803.14961, 0, -7.8740157),
		simple_point(0, 0, -7.8740157))));

	std::vector<space> spaces(1, space(s_info, &c));
	std::vector<element> elements(1, element(e_info, &c));
	std::vector<block> blocks = blocking::build_blocks(elements, &c);

	auto oriented_blocks = get_blocks_by_orientation(blocks);

	auto space_faces = get_space_faces_by_orientation(spaces, &c);
	for (auto o = space_faces.begin(); o != space_faces.end(); ++o) {
		if (o->first->direction() == direction_3(0, 0, 1)) {
			auto g = create_stacking_graph(&o->second, oriented_blocks[o->first], g_opts.equality_tolerance);
			auto vertices = boost::vertices(g);
			EXPECT_EQ(3, std::distance(vertices.first, vertices.second));
			auto edges = boost::edges(g);
			EXPECT_EQ(1, std::distance(edges.first, edges.second));
			return;
		}
	}
	ADD_FAILURE() << "Couldn't find stacking orientation <0, 0, 1>";
}

TEST(Stacking, FloorAndRoom) {
	equality_context c(g_opts.equality_tolerance);

	space_info * s_info = create_space("space", create_ext(0, 0, 1, 307.08661, create_face(5,
		simple_point(0, 0, 0),
		simple_point(393.70079, 0, 0),
		simple_point(393.70079, 387.79528, 0),
		simple_point(0, 387.79528, 0),
		simple_point(0, 0, 0))));

	element_info * e_info = create_element("floor", SLAB, 1, create_ext(0, 0, 1, 7.8740157, create_face(5,
		simple_point(0, 0, -7.8740157),
		simple_point(0, -8, -7.8740157),
		simple_point(803.14961, -8, -7.8740157),
		simple_point(803.14961, 401, -7.8740157),
		simple_point(0, 401, -7.8740157))));

	std::vector<space> spaces(1, space(s_info, &c));
	std::vector<element> elements(1, element(e_info, &c));

	auto stacks = stacking::build_stacks(blocking::build_blocks(elements, &c), spaces, 3000, &c);
	ASSERT_EQ(1, stacks.size());
	
	std::vector<std::unique_ptr<surface>> surfaces;
	stacks.front().to_surfaces(std::back_inserter(surfaces));
	ASSERT_EQ(1, surfaces.size());

	surface * s = surfaces.front().get();

	EXPECT_EQ(s->bounded_space().original_info(), s_info);
	EXPECT_EQ(1, s->material_layers().size());
	EXPECT_EQ(nullptr, s->other_side());
	EXPECT_EQ(nullptr, s->parent());
	EXPECT_FALSE(s->is_virtual());
	EXPECT_FALSE(s->is_fenestration());
	EXPECT_TRUE(s->is_external());
	EXPECT_FALSE(s->has_other_side());
	EXPECT_FALSE(s->shares_space_with_other_side());

	std::vector<polygon_with_holes_3> geom = s->geometry().to_3d();
	ASSERT_EQ(1, geom.size());
	std::vector<point_3> target_geom;
	target_geom.push_back(c.request_point(0, 0, 0));
	target_geom.push_back(c.request_point(0, 387.79528, 0));
	target_geom.push_back(c.request_point(393.70079, 387.79528, 0));
	target_geom.push_back(c.request_point(393.70079, 0, 0));
	EXPECT_EQ(geom.front(), polygon_with_holes_3(target_geom, std::vector<std::vector<point_3>>()));
}

TEST(Stacking, SecondLevel) {
	equality_context c(g_opts.equality_tolerance);

	face f;

	f = create_face(4,
		simple_point(20, 25, 157),
		simple_point(20, 125, 157),
		simple_point(120, 125, 157),
		simple_point(120, 25, 157));
	oriented_area ceiling(simple_face(f, &c), &c);

	f = create_face(4,
		simple_point(20, 25, 164),
		simple_point(120, 25, 164),
		simple_point(120, 125, 164),
		simple_point(20, 125, 164));
	oriented_area floor(simple_face(f, &c), &c);

	element_info * e_info = create_element("dummy element", SLAB, 1, create_ext(0, 0, 1, 7, f));
	std::vector<element> elements(1, element(e_info, &c));
	std::vector<block> blocks(1, block(ceiling, floor, elements.front()));
	block b(ceiling, floor, elements.front());

	space_info * s_lower = create_space("lower space", create_ext(0, 0, 1, 150, create_face(4,
		simple_point(30, 35, 7),
		simple_point(110, 35, 7),
		simple_point(110, 115, 7),
		simple_point(30, 115, 7))));
	space_info * s_upper = create_space("upper space", create_ext(0, 0, 1, 150, create_face(4,
		simple_point(30, 35, 164),
		simple_point(110, 35, 164),
		simple_point(110, 115, 164),
		simple_point(30, 115, 164))));

	std::vector<space> spaces;
	spaces.push_back(space(s_lower, &c));
	spaces.push_back(space(s_upper, &c));

	auto stacks = stacking::build_stacks(blocks, spaces, 3000, &c);
	EXPECT_EQ(1, stacks.size());
}

TEST(Stacking, EmptyAfterFaceSplit) {
	equality_context c(g_opts.equality_tolerance);

	face f;
	space_info * s_info;

	f = create_face(4,
		simple_point(10, 15, 1),
		simple_point(20, 15, 1),
		simple_point(20, 35, 1),
		simple_point(10, 15, 1));
	s_info = create_space("lower space", create_ext(0, 0, -1, 100, f));
	space lower_space(s_info, &c);
	oriented_area lower_face_geom(simple_face(f, &c), &c);

	f = create_face(4,
		simple_point(40, 15, 10), 
		simple_point(40, 35, 10),
		simple_point(60, 35, 10),
		simple_point(60, 15, 10));
	s_info = create_space("upper space", create_ext(0, 0, 1, 100, f));
	space upper_space(s_info, &c);
	oriented_area upper_face_geom(simple_face(f, &c), &c);

	std::vector<space_face> faces;
	faces.push_back(space_face(&lower_space, lower_face_geom));
	faces.push_back(space_face(&upper_space, upper_face_geom));

	f = create_face(4,
		simple_point(15, 15, 1),
		simple_point(15, 35, 1),
		simple_point(50, 35, 1),
		simple_point(50, 15, 1));
	oriented_area lower_block(simple_face(f, &c), &c);
	f = create_face(4,
		simple_point(15, 15, 10),
		simple_point(50, 15, 10),
		simple_point(50, 35, 10),
		simple_point(15, 35, 10));
	oriented_area upper_block(simple_face(f, &c), &c);
	element_info * e_info = create_element("element", UNKNOWN, 1, create_ext(0, 0, -1, 9, f));
	element dummy(e_info, &c);

	block b(lower_block, upper_block, dummy);

	std::vector<const block *> blocks(1, &b);

	stacking_graph g = create_stacking_graph(&faces, blocks, g_opts.equality_tolerance);

	std::vector<blockstack> stacks;
	begin_traversal(g, 0, std::get<0>(c.request_orientation(direction_3(0, 0, 1))), 300, g_opts.equality_tolerance, std::back_inserter(stacks));

	ASSERT_EQ(1, stacks.size());
	EXPECT_FALSE(stacks.front().stack_area().is_empty());
}

} // namespace

} // namespace impl

} // namespace stacking