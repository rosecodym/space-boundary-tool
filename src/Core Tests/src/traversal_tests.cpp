#include "precompiled.h"

#include <gtest/gtest.h>

#include "common.h"
#include "equality_context.h"
#include "identify_transmission.h"
#include "oriented_area.h"
#include "simple_face.h"
#include "space.h"
#include "transmission_information.h"

namespace traversal {

namespace impl {

TEST(Traversal, SimpleCase) {
	equality_context c(0.01);
	oriented_area b1(simple_face(create_face(4,
		simple_point(10, 0, 0),
		simple_point(10, 20, 0),
		simple_point(10, 20, 5),
		simple_point(10, 0, 5)), false, &c), &c);
	oriented_area b2(simple_face(create_face(4,
		simple_point(9.8, 0, 5),
		simple_point(9.8, 20, 5),
		simple_point(9.8, 20, 0),
		simple_point(9.8, 0, 0)), false, &c), &c);
	orientation * o = std::get<0>(c.request_orientation(direction_3(1, 0, 0)));
	space sp1(create_dummy_space(), &c);
	space sp2(create_dummy_space(), &c);
	auto e_info = create_dummy_element();
	element e(e_info, &c);
	std::vector<const block *> blocks;
	blocks.push_back(new block(b1, b2, e));
	std::vector<space_face> sfaces;
	sfaces.push_back(space_face(&sp1, b1.reverse()));
	sfaces.push_back(space_face(&sp2, b2.reverse()));
	std::vector<transmission_information> res;

	process_orientation(
		&sfaces,
		blocks,
		o,
		0.5,
		c,
		std::back_inserter(res));
	EXPECT_EQ(1, res.size());
}

} // namespace impl

} // namespace traversal