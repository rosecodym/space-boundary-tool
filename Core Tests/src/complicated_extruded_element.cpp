#include "precompiled.h"

#include <gtest/gtest.h>

#include "build_blocks.h"
#include "common.h"
#include "element.h"
#include "equality_context.h"
#include "halfblocks_for_base.h"
#include "is_hexahedral_prismatoid.h"
#include "is_right_cuboid.h"
#include "surface.h"
#include "surface_pair.h"

namespace blocking {

namespace impl {

namespace {

class ComplicatedExtrudedElement : public ::testing::Test {
protected:

	boost::optional<element> e;
	equality_context c;

	ComplicatedExtrudedElement() : c(0.01) {
		e = element(create_element("complicated extruded element", SLAB, 1, create_ext(0, 0, 1, 300, create_face(15,
			simple_point(0.0,		0.0,	0.0),
			simple_point(8250,		0.0,	0.0),
			simple_point(8250,		-2105,	0.0),
			simple_point(12120.109,	-2105,	0.0),
			simple_point(12120.109,	-4050,	0.0),
			simple_point(18195.109,	-4050,	0.0),
			simple_point(18195.109,	-8200,	0.0),
			simple_point(17181.249,	-8200,	0.0),
			simple_point(11991.013,	-29200,	0.0),
			simple_point(0.0,		-29200,	0.0),
			simple_point(0.0,		-28000,	0.0),
			simple_point(2050,		-28000,	0.0),
			simple_point(2050,		-8250,	0.0),
			simple_point(0.0,		-8250,	0.0),
			simple_point(0.0,		0.0,	0.0)))), &c);

	}

};

TEST_F(ComplicatedExtrudedElement, FacesCorrect) {
	auto faces = e->geometry().oriented_faces(&c);
	int x_bottom = 0;
	int x_middle_lower = 0;
	int x_left_lower = 0;
	int x_left_upper = 0;
	int x_top = 0;
	int y_left_lower = 0;
	int y_left_middle = 0;
	int y_left_upper = 0;
	int y_top_small = 0;
	int y_right = 0;
	int y_right_small = 0;
	int y_middle_small = 0;
	int z_lower = 0;
	int z_upper = 0;
	int diagonal = 0;
	int unaccounted_for = 0;
	boost::for_each(faces, [&](const oriented_area & f) {
		if (CGAL::is_zero(f.orientation().dx()) && CGAL::is_zero(f.orientation().dy())) {
			if (f.sense() && f.height() == 300) { ++z_upper; }
			else if (!f.sense() && f.height() == 0) { ++z_lower; }
			else { ++unaccounted_for; }
		}
		else if (CGAL::is_zero(f.orientation().dx()) && CGAL::is_zero(f.orientation().dz())) {
			if (f.sense() && f.height() == 0) { ++y_left_lower; }
			else if (f.sense() && f.height() == -2105) { ++y_left_middle; }
			else if (f.sense() && f.height() == -4050) { ++y_left_upper; }
			else if (!f.sense() && f.height() == -8200) { ++y_top_small; }
			else if (!f.sense() && f.height() == -29200) { ++y_right; }
			else if (f.sense() && f.height() == -28000) { ++y_right_small; }
			else if (!f.sense() && f.height() == -8250) { ++y_middle_small; }
			else { ++unaccounted_for; }
		}
		else if (CGAL::is_zero(f.orientation().dy()) && CGAL::is_zero(f.orientation().dz())) {
			if (f.sense() && f.height() == 8250) { ++x_left_lower; }
			else if (f.sense() && f.height() == 12120.109) { ++x_left_upper; }
			else if (f.sense() && f.height() == 18195.109) { ++x_top; }
			else if (!f.sense() && f.height() == 0) { ++x_bottom; }
			else if (!f.sense() && f.height() == 2050) { ++x_middle_lower; }
			else { ++unaccounted_for; }
		}
		else if (CGAL::is_zero(f.orientation().dz())) {
			++diagonal;
		}
		else {
			++unaccounted_for;
		}
	});
	EXPECT_EQ(16, faces.size());
	EXPECT_EQ(2, x_bottom);
	EXPECT_EQ(1, x_left_lower);
	EXPECT_EQ(1, x_left_upper);
	EXPECT_EQ(1, x_top);
	EXPECT_EQ(1, x_middle_lower);
	EXPECT_EQ(1, y_left_lower);
	EXPECT_EQ(1, y_left_middle);
	EXPECT_EQ(1, y_left_upper);
	EXPECT_EQ(1, y_top_small);
	EXPECT_EQ(1, y_right);
	EXPECT_EQ(1, y_right_small);
	EXPECT_EQ(1, y_middle_small);
	EXPECT_EQ(1, z_upper);
	EXPECT_EQ(1, z_lower);
	EXPECT_EQ(1, diagonal);
	EXPECT_EQ(0, unaccounted_for);
}

TEST_F(ComplicatedExtrudedElement, OrientationCounts) {
	auto faces = e->geometry().oriented_faces(&c);
	int x_count = 0;
	int y_count = 0;
	int z_count = 0;
	int other = 0;
	orientation x_axis(direction_3(1, 0, 0));
	orientation y_axis(direction_3(0, 1, 0));
	orientation z_axis(direction_3(0, 0, 1));
	for (size_t i = 0; i < faces.size(); ++i) {
		if (orientation::are_parallel(faces[i].orientation(), x_axis)) { ++x_count; }
		else if (orientation::are_parallel(faces[i].orientation(), y_axis)) { ++y_count; }
		else if (orientation::are_parallel(faces[i].orientation(), z_axis)) { ++z_count; }
		else { ++other; }
	}
	EXPECT_EQ(6, x_count);
	EXPECT_EQ(7, y_count);
	EXPECT_EQ(2, z_count);
	EXPECT_EQ(1, other);
	EXPECT_EQ(faces.size(), x_count + y_count + z_count + other);
}

TEST_F(ComplicatedExtrudedElement, BlockCountCorrect) {
	EXPECT_EQ(15, build_blocks_for(*e, &c).size());
}

} // namespace

} // namespace impl

} // namespace blocking