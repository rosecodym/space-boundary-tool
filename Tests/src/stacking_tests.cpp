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