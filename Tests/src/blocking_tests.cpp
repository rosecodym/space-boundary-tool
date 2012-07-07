#include "precompiled.h"

#include <gtest/gtest.h>

#include "build_blocks.h"
#include "common.h"
#include "element.h"
#include "halfblocks_for_base.h"
#include "link_halfblocks.h"
#include "oriented_area.h"
#include "sbt-core.h"
#include "simple_face.h"
#include "surface_pair.h"

extern sb_calculation_options g_opts;

namespace blocking {

namespace impl {

namespace {

TEST(Blocking, ParallelPairHalfblocks) {
	equality_context c(g_opts.equality_tolerance);

	std::vector<oriented_area> surfaces;

	face f;

	f = create_face(4, 
		simple_point(0, 0, 0),
		simple_point(8250, 0, 0),
		simple_point(8250, 0, 300),
		simple_point(0, 0, 300));

	surfaces.push_back(oriented_area(simple_face(f, &c), &c));
	ASSERT_FALSE(surfaces.back().sense()) << "(Near)";
	ASSERT_EQ(0, surfaces.back().height()) << "(Near)";
	ASSERT_EQ(direction_3(0, 1, 0), surfaces.back().orientation().direction()) << "(Near)";

	f = create_face(4,
		simple_point(0, 8250, 300),
		simple_point(8250, 8250, 300),
		simple_point(8250, 8250, 0),
		simple_point(0, 8250, 0));

	surfaces.push_back(oriented_area(simple_face(f, &c), &c));
	ASSERT_TRUE(surfaces.back().sense()) << "(Far)";
	ASSERT_EQ(8250, surfaces.back().height()) << "(Far)";
	ASSERT_EQ(direction_3(0, 1, 0), surfaces.back().orientation().direction()) << "(Far)";

	std::vector<oriented_area> halfblocks;

	auto rels = build_relations_grid(surfaces, &c);

	halfblocks.clear();
	halfblocks_for_base(rels, 0, &c, std::back_inserter(halfblocks));
	EXPECT_EQ(1, halfblocks.size()) << "(Near)";

	halfblocks.clear();
	halfblocks_for_base(rels, 1, &c, std::back_inserter(halfblocks));
	EXPECT_EQ(1, halfblocks.size()) << "(Far)";
}

TEST(Blocking, TwoStairs) {
	equality_context c(g_opts.equality_tolerance);

	std::vector<oriented_area> surfaces;

	face f;

	f = create_face(4,
		simple_point(0, 0, 0),
		simple_point(8250, 0, 0),
		simple_point(8250, 0, 300),
		simple_point(0, 0, 300));

	surfaces.push_back(oriented_area(simple_face(f, &c), &c));
	ASSERT_FALSE(surfaces.back().sense()) << "(Base)";
	ASSERT_EQ(0, surfaces.back().height()) << "(Base)";
	ASSERT_EQ(direction_3(0, 1, 0), surfaces.back().orientation().direction()) << "(Base)";

	f = create_face(4,
		simple_point(0, 8250, 300),
		simple_point(4050, 8250, 300),
		simple_point(4050, 8250, 0),
		simple_point(0, 8250, 0));

	surfaces.push_back(oriented_area(simple_face(f, &c), &c));
	ASSERT_TRUE(surfaces.back().sense()) << "(Near)";
	ASSERT_EQ(8250, surfaces.back().height()) << "(Near)";
	ASSERT_EQ(direction_3(0, 1, 0), surfaces.back().orientation().direction()) << "(Near)";

	f = create_face(4,
		simple_point(4050, 18195.109, 300),
		simple_point(8250, 18195.109, 300),
		simple_point(8250, 18195.109, 0),
		simple_point(4050, 18195.109, 0));

	surfaces.push_back(oriented_area(simple_face(f, &c), &c));
	ASSERT_TRUE(surfaces.back().sense()) << "(Far)";
	ASSERT_EQ(18195.109, surfaces.back().height()) << "(Far)";
	ASSERT_EQ(direction_3(0, 1, 0), surfaces.back().orientation().direction()) << "(Far)";

	relations_grid rels = build_relations_grid(surfaces, &c);

	const surface_pair & bb = rels[0][0];
	EXPECT_TRUE(bb.is_self());
	EXPECT_FALSE(bb.contributes_to_envelope());

	const surface_pair & bn = rels[0][1];
	EXPECT_FALSE(bn.is_self());
	EXPECT_FALSE(bn.are_perpendicular());
	EXPECT_TRUE(bn.are_parallel());
	EXPECT_TRUE(bn.base().sense() == !bn.other().sense());
	EXPECT_TRUE(bn.base().height() > bn.other().height() == bn.base().sense());
	EXPECT_TRUE(area::do_intersect(bn.base().area_2d(), bn.other().area_2d()));
	EXPECT_TRUE(bn.contributes_to_envelope());

	const surface_pair & bf = rels[0][2];
	EXPECT_FALSE(bf.is_self());
	EXPECT_FALSE(bf.are_perpendicular());
	EXPECT_TRUE(bf.are_parallel());
	EXPECT_TRUE(bf.base().sense() == !bf.other().sense());
	EXPECT_TRUE(bf.base().height() > bf.other().height() == bf.base().sense());
	EXPECT_TRUE(area::do_intersect(bf.base().area_2d(), bf.other().area_2d()));
	EXPECT_TRUE(bf.contributes_to_envelope());

	const surface_pair & nb = rels[1][0];
	EXPECT_FALSE(nb.is_self());
	EXPECT_FALSE(nb.are_perpendicular());
	EXPECT_TRUE(nb.are_parallel());
	EXPECT_TRUE(nb.base().sense() == !nb.other().sense());
	EXPECT_TRUE(nb.base().height() > nb.other().height() == nb.base().sense());
	EXPECT_TRUE(area::do_intersect(nb.base().area_2d(), nb.other().area_2d())) <<
		(nb.base().area_2d().print(), "") << (nb.other().area_2d().print(), "");
	EXPECT_TRUE(nb.contributes_to_envelope());

	const surface_pair & nn = rels[1][1];
	EXPECT_TRUE(nn.is_self());
	EXPECT_FALSE(nn.contributes_to_envelope());

	const surface_pair & nf = rels[1][2];
	EXPECT_FALSE(nf.is_self());
	EXPECT_FALSE(nf.are_perpendicular());
	EXPECT_TRUE(nf.are_parallel());
	EXPECT_FALSE(nf.base().sense() == !nf.other().sense());
	EXPECT_FALSE(nf.contributes_to_envelope());

	const surface_pair & fb = rels[2][0];
	EXPECT_FALSE(fb.is_self());
	EXPECT_FALSE(fb.are_perpendicular());
	EXPECT_TRUE(fb.are_parallel());
	EXPECT_TRUE(fb.base().sense() == !fb.other().sense());
	EXPECT_TRUE(fb.base().height() > fb.other().height() == fb.base().sense());
	EXPECT_TRUE(area::do_intersect(fb.base().area_2d(), fb.other().area_2d()));
	EXPECT_TRUE(fb.contributes_to_envelope());

	const surface_pair & fn = rels[2][1];
	EXPECT_FALSE(fn.is_self());
	EXPECT_FALSE(fn.are_perpendicular());
	EXPECT_TRUE(fn.are_parallel());
	EXPECT_FALSE(fn.base().sense() == !fn.other().sense());
	EXPECT_FALSE(fn.contributes_to_envelope());

	const surface_pair & ff = rels[2][2];
	EXPECT_TRUE(ff.is_self());
	EXPECT_FALSE(ff.contributes_to_envelope());

	std::vector<oriented_area> halfblocks;

	halfblocks_for_base(rels, 0, &c, std::back_inserter(halfblocks));
	EXPECT_EQ(2, halfblocks.size()) << "(Base)";

	halfblocks.clear();
	halfblocks_for_base(rels, 1, &c, std::back_inserter(halfblocks));
	EXPECT_EQ(1, halfblocks.size()) << "(Near)";

	halfblocks.clear();
	halfblocks_for_base(rels, 2, &c, std::back_inserter(halfblocks));
	EXPECT_EQ(1, halfblocks.size()) << "(Far)";
}

TEST(Blocking, ThreeStairsHalfblocks) {
	equality_context c(g_opts.equality_tolerance);

	std::vector<oriented_area> surfaces;

	face f;

	f = create_face(4,
		simple_point(0, 0, 0),
		simple_point(8250, 0, 0),
		simple_point(8250, 0, 300),
		simple_point(0, 0, 300));

	surfaces.push_back(oriented_area(simple_face(f, &c), &c));
	ASSERT_FALSE(surfaces.back().sense()) << "(Base)";
	ASSERT_EQ(0, surfaces.back().height()) << "(Base)";
	ASSERT_EQ(direction_3(0, 1, 0), surfaces.back().orientation().direction()) << "(Base)";

	f = create_face(4,
		simple_point(0, 8250, 300),
		simple_point(2105, 8250, 300),
		simple_point(2105, 8250, 0),
		simple_point(0, 8250, 0));

	surfaces.push_back(oriented_area(simple_face(f, &c), &c));
	ASSERT_TRUE(surfaces.back().sense()) << "(Near)";
	ASSERT_EQ(8250, surfaces.back().height()) << "(Near)";
	ASSERT_EQ(direction_3(0, 1, 0), surfaces.back().orientation().direction()) << "(Near)";

	f = create_face(4,
		simple_point(2105, 12120.109, 300),
		simple_point(4050, 12120.109, 300),
		simple_point(4050, 12120.109, 0),
		simple_point(2105, 12120.109, 0));

	surfaces.push_back(oriented_area(simple_face(f, &c), &c));
	ASSERT_TRUE(surfaces.back().sense()) << "(Mid)";
	ASSERT_EQ(12120.109, surfaces.back().height()) << "(Mid)";
	ASSERT_EQ(direction_3(0, 1, 0), surfaces.back().orientation().direction()) << "(Mid)";

	f = create_face(4,
		simple_point(4050, 18195.109, 300),
		simple_point(8250, 18195.109, 300),
		simple_point(8250, 18195.109, 0),
		simple_point(4050, 18195.109, 0));

	surfaces.push_back(oriented_area(simple_face(f, &c), &c));
	ASSERT_TRUE(surfaces.back().sense()) << "(Far)";
	ASSERT_EQ(18195.109, surfaces.back().height()) << "(Far)";
	ASSERT_EQ(direction_3(0, 1, 0), surfaces.back().orientation().direction()) << "(Far)";

	relations_grid rels = build_relations_grid(surfaces, &c);
	std::vector<oriented_area> halfblocks;

	halfblocks_for_base(rels, 0, &c, std::back_inserter(halfblocks));
	EXPECT_EQ(3, halfblocks.size()) << "(Base)";

	halfblocks.clear();
	halfblocks_for_base(rels, 1, &c, std::back_inserter(halfblocks));
	EXPECT_EQ(1, halfblocks.size()) << "(Near)";

	halfblocks.clear();
	halfblocks_for_base(rels, 2, &c, std::back_inserter(halfblocks));
	EXPECT_EQ(1, halfblocks.size()) << "(Mid)";

	halfblocks.clear();
	halfblocks_for_base(rels, 3, &c, std::back_inserter(halfblocks));
	EXPECT_EQ(1, halfblocks.size()) << "(Far)";
}

TEST(Blocking, ThreeStairsElement) { 
	equality_context c(g_opts.equality_tolerance);

	element_info * e_info = create_element("element", UNKNOWN, 1, create_ext(0, 0, 1, 300, create_face(8,
		simple_point(0, 0, 0),
		simple_point(0, 8250, 0),
		simple_point(2105, 8250, 0),
		simple_point(2105, 12120.109, 0),
		simple_point(4050, 12120.109, 0),
		simple_point(4050, 18195.109, 0),
		simple_point(8200, 18195.109, 0),
		simple_point(8200, 0, 0))));

	element e(e_info, &c);
	auto blocks = build_blocks_for(e, &c);
	ASSERT_EQ(10, e.faces(&c).size());
	EXPECT_EQ(7, blocks.size());
}

TEST(Blocking, NonparallelPairHalfblocksForSmallerBase) {
	equality_context c(g_opts.equality_tolerance);

	std::vector<oriented_area> surfaces;

	face f;

	f = create_face(4,
		simple_point(-8250, 2050, 0),
		simple_point(-28000, 2050, 0),
		simple_point(-28000, 2050, 300),
		simple_point(-8250, 2050, 300));
	surfaces.push_back(oriented_area(simple_face(f, &c), &c));

	f = create_face(4,
		simple_point(17181.249, -8200, 300),
		simple_point(11991.013, -29200, 300),
		simple_point(11991.013, -29200, 0),
		simple_point(17181.249, -8200, 0));
	surfaces.push_back(oriented_area(simple_face(f, &c), &c));

	auto rels = build_relations_grid(surfaces, &c);

	std::vector<oriented_area> halfblocks;
	halfblocks_for_base(rels, 0, &c, std::back_inserter(halfblocks));
	EXPECT_EQ(1, halfblocks.size());
}

TEST(Blocking, HalfblocksWithNonParallelOthers) {
	equality_context c(g_opts.equality_tolerance);
	
	std::vector<oriented_area> surfaces;

	face f;

	f = create_face(4,
		simple_point(4050, 12120.109, 300),
		simple_point(4050, 18195.109, 300),
		simple_point(4050, 18195.109, 0),
		simple_point(4050, 12120.109, 0));
	surfaces.push_back(oriented_area(simple_face(f, &c), &c));

	ASSERT_EQ(4, surfaces.back().area_2d().outer_bound().size());

	f = create_face(4,
		simple_point(8200, 17181.249, 0),
		simple_point(8200, 18195.109, 0),
		simple_point(8200, 18195.109, 300),
		simple_point(8200, 17181.249, 300));
	surfaces.push_back(oriented_area(simple_face(f, &c), &c));

	ASSERT_EQ(4, surfaces.back().area_2d().outer_bound().size());

	f = create_face(4,
		simple_point(8200, 17181.249, 300),
		simple_point(29200, 11991.913, 300),
		simple_point(29200, 11991.913, 0),
		simple_point(8200, 17181.249, 0));
	surfaces.push_back(oriented_area(simple_face(f, &c), &c));

	ASSERT_EQ(4, surfaces.back().area_2d().outer_bound().size());

	auto rels = build_relations_grid(surfaces, &c);

	std::vector<oriented_area> halfblocks;
	halfblocks_for_base(rels, 0, &c, std::back_inserter(halfblocks));
	EXPECT_EQ(2, halfblocks.size());
}

TEST(Blocking, Boot) {
	equality_context c(g_opts.equality_tolerance);

	element_info * e_info = create_element("boot element", UNKNOWN, 1, create_ext(0, 0, 1, 300, create_face(5,
		simple_point(4050, 12120.109, 0),
		simple_point(4050, 18195.109, 0),
		simple_point(8200, 18195.109, 0),
		simple_point(8200, 17181.249, 0),
		simple_point(29200, 5000, 0))));
	
	element e(e_info, &c);

	auto blocks = build_blocks_for(e, &c);
	EXPECT_EQ(6, blocks.size());
}

TEST(Blocking, SinglePairLink) {
	equality_context c(g_opts.equality_tolerance);

	face f;

	f = create_face(4,
		simple_point(4050, 12120.109, 300),
		simple_point(4050, 18195.109, 300),
		simple_point(4050, 18195.109, 0),
		simple_point(4050, 12120.109, 0));
	oriented_area a(simple_face(f, &c), &c);

	f = create_face(4,
		simple_point(8200, 12120.109, 0),
		simple_point(8200, 18195.109, 0),
		simple_point(8200, 18195.109, 300),
		simple_point(8200, 12120.109, 300));
	oriented_area b(simple_face(f, &c), &c);

	ASSERT_TRUE(oriented_area::could_form_block(a, b));

	std::list<oriented_area> surfaces;
	surfaces.push_back(a);
	surfaces.push_back(b);

	element_info * dummy_element = create_element("dummy element", UNKNOWN, 1, create_ext(0, 0, 1, 100, create_face(4,
		simple_point(0, 0, 0),
		simple_point(1, 0, 0),
		simple_point(1, 1, 0),
		simple_point(0, 1, 0))));
	element dummy(dummy_element, &c);

	std::vector<block> blocks;
	link_halfblocks(std::move(surfaces), dummy, std::back_inserter(blocks));
	EXPECT_EQ(1, blocks.size());
}

TEST(Blocking, RightCuboid) {
	equality_context c(g_opts.equality_tolerance);

	element_info * e = create_element("column", COLUMN, 1, create_ext(0, 0, 1, 4000, create_face(4,
		simple_point(0, 0, 0),
		simple_point(0, 255, 0),
		simple_point(255, 255, 0),
		simple_point(255, 0, 0))));

	element col(e, &c);
	std::vector<element> elements(1, col);

	std::vector<std::shared_ptr<surface>> surfaces;
	auto blocks = blocking::build_blocks(elements, &c);
	EXPECT_EQ(3, blocks.size());
}

TEST(Blocking, ColumnToLegacySurfaces) {
	equality_context c(g_opts.equality_tolerance);

	element_info * e = create_element("column", COLUMN, 1, create_ext(0, 0, 1, 4000, create_face(4,
		simple_point(0, 0, 0),
		simple_point(0, 255, 0),
		simple_point(255, 255, 0),
		simple_point(255, 0, 0))));

	element col(e, &c);
	std::vector<element> elements(1, col);

	std::vector<std::shared_ptr<surface>> surfaces;
	auto blocks = blocking::build_blocks(elements, &c);
	boost::for_each(blocks, [&surfaces](const block & b) { b.as_surfaces(std::back_inserter(surfaces)); });

	auto surface_string = [](std::shared_ptr<surface> & s) -> std::string { 
		std::stringstream ss;
		ss << "<" <<
			CGAL::to_double(s->geometry().orientation().dx()) << ", " <<
			CGAL::to_double(s->geometry().orientation().dy()) << ", " <<
			CGAL::to_double(s->geometry().orientation().dz()) << "> @ " <<
			CGAL::to_double(s->geometry().height()) <<
			(s->geometry().sense() ? " (not reversed)" : " (reversed)");
		return ss.str();
	};

	auto all_surfaces = [&surfaces, &surface_string]() -> std::string {
		std::stringstream ss;
		for (auto s = surfaces.begin(); s != surfaces.end(); ++s) {
			ss << surface_string(*s) << std::endl;
		}
		return ss.str();
	};

	EXPECT_EQ(6, surfaces.size());

	auto found_and_sense = [](bool found, bool sense_ok) { return found && sense_ok; };

	auto top = boost::find_if(surfaces, [&c](std::shared_ptr<surface> & s) { 
		return s->geometry().orientation().direction() == direction_3(0, 0, 1) && s->geometry().height() == c.request_height(4000);
	});
	bool top_found = top != surfaces.end();
	bool top_sense = top_found && (*top)->geometry().sense();
	EXPECT_PRED2(found_and_sense, top_found, top_sense);

	auto bottom = boost::find_if(surfaces, [&c](std::shared_ptr<surface> & s) { 
		return s->geometry().orientation().direction() == direction_3(0, 0, 1) && s->geometry().height() == c.request_height(0);
	});
	bool bottom_found = bottom != surfaces.end();
	bool bottom_sense = bottom_found && !(*bottom)->geometry().sense();
	EXPECT_PRED2(found_and_sense, bottom_found, bottom_found);

	auto left = boost::find_if(surfaces, [&c](std::shared_ptr<surface> & s) { 
		return s->geometry().orientation().direction() == direction_3(1, 0, 0) && s->geometry().height() == c.request_height(0);
	});
	bool left_found = left != surfaces.end();
	bool left_sense = left_found && !(*left)->geometry().sense();
	EXPECT_PRED2(found_and_sense, left_found, left_sense);

	auto right = boost::find_if(surfaces, [&c](std::shared_ptr<surface> & s) { 
		return s->geometry().orientation().direction() == direction_3(1, 0, 0) && s->geometry().height() == c.request_height(225);
	});
	bool right_found = right != surfaces.end();
	bool right_sense = right_found && (*right)->geometry().sense();
	EXPECT_PRED2(found_and_sense, right_found, right_sense);

	auto front = boost::find_if(surfaces, [&c](std::shared_ptr<surface> & s) { 
		return s->geometry().orientation().direction() == direction_3(0, 1, 0) && s->geometry().height() == c.request_height(0);
	});
	bool front_found = front != surfaces.end();
	bool front_sense = front_found && !(*front)->geometry().sense();
	EXPECT_PRED2(found_and_sense, front_found, front_sense);

	auto back = boost::find_if(surfaces, [&c](std::shared_ptr<surface> & s) { 
		return s->geometry().orientation().direction() == direction_3(0, 1, 0) && s->geometry().height() == c.request_height(225);
	});
	bool back_found = back != surfaces.end();
	bool back_sense = back_found && (*back)->geometry().sense();
	EXPECT_PRED2(found_and_sense, back_found, back_sense);

	if (surfaces.size() != 6 ||
		top == surfaces.end() || !(*top)->geometry().sense() ||
		bottom == surfaces.end() || (*bottom)->geometry().sense() || 
		left == surfaces.end() || (*left)->geometry().sense() ||
		right == surfaces.end() || !(*right)->geometry().sense() ||
		front == surfaces.end() || (*front)->geometry().sense() ||
		back == surfaces.end() || !(*back)->geometry().sense())
	{
		printf(all_surfaces().c_str());
	}
}

} // namespace

} // namespace impl

} // namespace blocking