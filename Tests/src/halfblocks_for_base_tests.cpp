#include "precompiled.h"

#include <gtest/gtest.h>

#include "halfblocks_for_base.h"

#include "common.h"
#include "equality_context.h"
#include "oriented_area.h"
#include "simple_face.h"

namespace blocking {

namespace impl {

namespace {

TEST(HalfblocksForBase, SingleParallelCountCorrect) {
	equality_context c(0.01);
	std::vector<oriented_area> surfaces;
	surfaces.push_back(oriented_area(simple_face(create_face(4, 
		simple_point(8200, 18000, 0),
		simple_point(8200, 18000, 300),
		simple_point(8200, 17000, 300),
		simple_point(8200, 17000, 0)), false, &c), &c));
	surfaces.push_back(oriented_area(simple_face(create_face(4, 
		simple_point(4050, 12000, 300),
		simple_point(4050, 18000, 300),
		simple_point(4050, 18000, 0),
		simple_point(4050, 12000, 0)), false, &c), &c));
	auto rels = surface_pair::build_relations_grid(surfaces, &c);
	ASSERT_EQ(surface_pair::NONE, rels[0][0].contributes_to_envelope());
	ASSERT_EQ(surface_pair::PARALLEL, rels[0][1].contributes_to_envelope());
	ASSERT_EQ(surface_pair::PARALLEL, rels[1][0].contributes_to_envelope());
	ASSERT_EQ(surface_pair::NONE, rels[1][1].contributes_to_envelope());
	ASSERT_EQ(4, rels[0][1].projected_other_area().size());
	std::vector<oriented_area> res;
	halfblocks_for_base(rels, 0, &c, std::back_inserter(res));
	EXPECT_EQ(1, res.size());
}
	
} // namespace

} // namespace impl

} // namespace blocking