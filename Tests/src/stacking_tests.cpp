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
		simple_point(0, 0, 0),
		simple_point(0, -409.44882, 0),
		simple_point(803.14961, -409.44882, 0),
		simple_point(803.14961, 0, 0),
		simple_point(0, 0, 0))));

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
		simple_point(0, 0, 0),
		simple_point(0, -409.44882, 0),
		simple_point(803.14961, -409.44882, 0),
		simple_point(803.14961, 0, 0),
		simple_point(0, 0, 0))));

	std::vector<space> spaces(1, space(s_info, &c));
	std::vector<element> elements(1, element(e_info, &c));

	auto stacks = stacking::build_stacks(blocking::build_blocks(elements, &c), spaces, 3000, &c);
	EXPECT_EQ(1, stacks.size());
}

} // namespace

} // namespace impl

} // namespace stacking