#include "precompiled.h"

#include <gtest/gtest.h>

#include "area.h"
#include "blockstack.h"
#include "common.h"
#include "equality_context.h"
#include "layer_information.h"
#include "oriented_area.h"
#include "sbt-core.h"
#include "space.h"
#include "surface.h"

extern sb_calculation_options g_opts;

namespace {

TEST(Blockstack, SimpleSecondLevelSurfaces) {
	equality_context c(g_opts.equality_tolerance);

	face f = create_face(4,
		simple_point(10, 0, 0),
		simple_point(10, 0, 9),
		simple_point(10, 15, 9),
		simple_point(10, 15, 0));

	element_info * e = create_element("dummy element", WALL, 1, create_ext(1, 0, 0, 3.5, f));
	space_info * s1 = create_space("dummy space 1", create_ext(-1, 0, 0, 20, f));
	space_info * s2 = create_space("dummy space 2", create_ext(1, 0, 0, 20, f)); // this geometry isn't correct but it doesn't matter for this test

	element ele(e, &c);
	space sp1(s1, &c);
	space sp2(s2, &c);

	oriented_area geom(simple_face(f, &c), &c);

	blockstack st(
		area(geom.area_2d()),
		std::vector<layer_information>(1, layer_information(10, 11.5, ele)), // materials 
		false, // base sense
		std::get<0>(c.request_orientation(direction_3(-1, 0, 0))), // orientation
		false, // external
		&sp1, // near space
		10, // near height
		boost::optional<const space *>(&sp2), // far space
		boost::optional<NT>(11.5)); // far height

	std::vector<std::unique_ptr<surface>> surfaces;
	st.to_surfaces(std::back_inserter(surfaces));

	EXPECT_EQ(2, surfaces.size());
	EXPECT_EQ(surfaces.end(), boost::find_if(surfaces, [](const std::unique_ptr<surface> & s) {
		return s->is_virtual();
	}));
}

TEST(Blockstack, FifthLevelSurface) {
		equality_context c(g_opts.equality_tolerance);

	face f = create_face(4,
		simple_point(10, 0, 0),
		simple_point(10, 0, 9),
		simple_point(10, 15, 9),
		simple_point(10, 15, 0));

	element_info * e = create_element("dummy element", WALL, 1, create_ext(1, 0, 0, 3.5, f)); // ignore the geometry, it's not relevant
	space_info * s1 = create_space("dummy space", create_ext(-1, 0, 0, 20, f));

	element ele(e, &c);
	space sp1(s1, &c);

	oriented_area geom(simple_face(f, &c), &c);

	blockstack st(
		area(geom.area_2d()),
		std::vector<layer_information>(1, layer_information(10, ele)), // materials 
		false, // base sense
		std::get<0>(c.request_orientation(direction_3(-1, 0, 0))), // orientation
		false, // external
		&sp1, // near space
		10); // near height

	std::vector<std::unique_ptr<surface>> surfaces;
	st.to_surfaces(std::back_inserter(surfaces));

	ASSERT_EQ(1, surfaces.size());
	EXPECT_FALSE(surfaces.front()->is_virtual());
	EXPECT_EQ(0, surfaces.front()->material_layers().size());
}

} // namespace