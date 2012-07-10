#include "precompiled.h"

#include <gtest/gtest.h>

#include "common.h"
#include "build_stacks.h"
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
		simple_point(0, 387.79528, 0))));

	std::vector<space> spaces(1, space(s_info, &c));

	auto faces = get_space_faces_by_orientation(spaces, &c);

	EXPECT_EQ(3, faces.size());
}

} // namespace

} // namespace impl

} // namespace stacking