#include "precompiled.h"

#include <gtest/gtest.h>

#include "is_hexahedral_prismatoid.h"

#include "common.h"
#include "equality_context.h"
#include "oriented_area.h"
#include "element.h"
#include "simple_face.h"
#include "surface_pair.h"

namespace blocking {

namespace impl {

namespace {

template <typename BlockRange>
size_t halfblock_count(const BlockRange & blocks) {
	return boost::count_if(blocks, [](const block & b) {
		return !b.heights().second;
	});
}

template <typename BlockRange>
size_t full_block_count(const BlockRange & blocks) {
	return boost::count_if(blocks, [](const block & b) {
		return b.heights().second;
	});
}

TEST(HexahedralPrismatoidCheck, ShortWide) {
	equality_context c(0.01);
	double max_distance = 1.0;

	face base = create_face(4,
		simple_point(2, 2, 2),
		simple_point(12, 2, 2),
		simple_point(10, 10, 2),
		simple_point(4, 10, 2));
	auto ext = create_ext(0, 0, 1, 0.5, base);
	element e(create_element("dummy", SLAB, 0, ext), &c);

	auto faces = e.faces(&c);

	relations_grid rels(boost::extents[faces.size()][faces.size()]);
	for (size_t i = 0; i < faces.size(); ++i) {
		for (size_t j = i; j < faces.size(); ++j) {
			rels[i][j] = surface_pair(
				faces[i], 
				faces[j], 
				&c, 
				max_distance);
			rels[j][i] = rels[i][j].opposite();
		}
	}

	std::vector<block> blocks;
	ASSERT_TRUE(is_hexahedral_prismatoid(
		rels, 
		faces.size(),
		e,
		max_distance,
		std::back_inserter(blocks)));
	EXPECT_EQ(1, full_block_count(blocks));
	EXPECT_EQ(4, halfblock_count(blocks));
}

} // namespace

} // namespace impl

} // namespace blocking