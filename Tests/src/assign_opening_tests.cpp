#include "precompiled.h"

#include <gtest/gtest.h>

#include "assign_openings.h"

#include "block.h"
#include "common.h"
#include "element.h"
#include "equality_context.h"
#include "oriented_area.h"
#include "space.h"
#include "surface.h"

namespace opening_assignment {

namespace impl {

TEST(PlaceOpeningBlock, Halfblock) {
	equality_context c(0.01);

	element e(create_dummy_element(), &c);

	block b(oriented_area(simple_face(create_face(4,
		simple_point(0, 0, 0),
		simple_point(-5, 0, 0),
		simple_point(-5, 1, 0),
		simple_point(0, 1, 0)), &c), &c), e);

	std::vector<std::unique_ptr<surface>> surfaces;
	auto res = place_opening_block(b, surfaces, 0.01);
	EXPECT_EQ(0, res.size());
}

TEST(PlaceOpeningBlock, MultispaceMatch) {
	equality_context c(0.01);

	space s(create_dummy_space(), &c);
	element e(create_dummy_element(), &c);
	std::vector<layer_information> dummy_layers;

	block b(
		oriented_area(simple_face(create_face(4,
			simple_point(-5, 0, 0),
			simple_point(5, 0, 0),
			simple_point(5, 0, 5),
			simple_point(-5, 0, 5)), &c), &c),
		oriented_area(simple_face(create_face(4,
			simple_point(-5, 1, 0),
			simple_point(-5, 1, 5),
			simple_point(5, 1, 5),
			simple_point(5, 1, 0)), &c), &c),
		e);

	surface sface_1(oriented_area(simple_face(create_face(4,
		simple_point(-10, 0, 0),
		simple_point(-10, 0, 5),
		simple_point(0, 0, 5),
		simple_point(0, 0, 0)), &c), &c), e, s, dummy_layers, false);
	surface sface_2(oriented_area(simple_face(create_face(4,
		simple_point(0, 0, 0),
		simple_point(0, 0, 5),
		simple_point(10, 0, 5),
		simple_point(10, 0, 0)), &c), &c), e, s, dummy_layers, false);
	surface sface_3(oriented_area(simple_face(create_face(4,
		simple_point(-10, 1, 5),
		simple_point(-10, 1, 0),
		simple_point(0, 1, 0),
		simple_point(0, 1, 5)), &c), &c), e, s, dummy_layers, false);
	surface sface_4(oriented_area(simple_face(create_face(4,
		simple_point(0, 1, 0),
		simple_point(10, 1, 0),
		simple_point(10, 1, 5),
		simple_point(0, 1, 5)), &c), &c), e, s, dummy_layers, false);

	std::vector<std::unique_ptr<surface>> surfaces;
	surfaces.push_back(std::unique_ptr<surface>(new surface(sface_1)));
	surfaces.push_back(std::unique_ptr<surface>(new surface(sface_2)));
	surfaces.push_back(std::unique_ptr<surface>(new surface(sface_3)));
	surfaces.push_back(std::unique_ptr<surface>(new surface(sface_4)));

	auto res = place_opening_block(b, surfaces, 0.01);
	EXPECT_EQ(4, res.size());
}

} // namespace impl

} // namespace opening_assignment