#include "precompiled.h"

#include <gtest/gtest.h>

#include "build_blocks.h"
#include "element.h"
#include "equality_context.h"
#include "halfblocks_for_base.h"
#include "is_hexahedral_prismatoid.h"
#include "is_right_cuboid.h"
#include "scan_for_degenerate_halfblocks.h"
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
		element_info info;
		strcpy(info.id, "complicated extruded element");
		info.type = SLAB;
		info.material = 1;
		set_to_extruded_area_solid(&info.geometry, 0.0, 0.0, 1.0, 300);
		face * a = get_area_handle(&info.geometry);
		set_void_count(a, 0);
		polyloop * outer = get_outer_boundary_handle(a);
		set_vertex_count(outer, 15);
		set_vertex(outer, 0, 0.0,		0.0,	0.0);
		set_vertex(outer, 1, 8250,		0.0,	0.0);
		set_vertex(outer, 2, 8250,		-2105,	0.0);
		set_vertex(outer, 3, 12120.109,	-2105,	0.0);
		set_vertex(outer, 4, 12120.109,	-4050,	0.0);
		set_vertex(outer, 5, 18195.109,	-4050,	0.0);
		set_vertex(outer, 6, 18195.109,	-8200,	0.0);
		set_vertex(outer, 7, 17181.249,	-8200,	0.0);
		set_vertex(outer, 8, 11991.013,	-29200,	0.0);
		set_vertex(outer, 9, 0.0,		-29200,	0.0);
		set_vertex(outer, 10, 0.0,		-28000,	0.0);
		set_vertex(outer, 11, 2050,		-28000,	0.0);
		set_vertex(outer, 12, 2050,		-8250,	0.0);
		set_vertex(outer, 13, 0.0,		-8250,	0.0);
		set_vertex(outer, 14, 0.0,		0.0,	0.0);
		e = element(&info, &c);
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

TEST_F(ComplicatedExtrudedElement, NotRightCuboid) {
	auto faces = e->geometry().oriented_faces(&c);
	auto rels = build_relations_grid(faces, &c);
	std::vector<block> dummy;
	EXPECT_FALSE(is_right_cuboid(rels, faces.size(), *e, std::back_inserter(dummy)));
}

TEST_F(ComplicatedExtrudedElement, NotHexahedralPrismatoid) {
	auto faces = e->geometry().oriented_faces(&c);
	auto rels = build_relations_grid(faces, &c);
	std::vector<block> dummy;
	EXPECT_FALSE(is_hexahedral_prismatoid(rels, faces.size(), *e, std::back_inserter(dummy)));
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

TEST_F(ComplicatedExtrudedElement, InitialDegenerateCheck) {
	auto faces = e->geometry().oriented_faces(&c);
	auto rels = build_relations_grid(faces, &c);
	std::vector<block> dummy;
	auto degenerate = scan_for_degenerate_halfblocks(rels, faces.size(), *e, std::back_inserter(dummy));
	EXPECT_EQ(1, degenerate.size());
	for (auto ix = degenerate.begin(); ix != degenerate.end(); ++ix) {
		const orientation & o = faces[*ix].orientation();
		bool x_zero = CGAL::is_zero(o.dx());
		bool y_zero = CGAL::is_zero(o.dy());
		bool z_zero = CGAL::is_zero(o.dz());
		EXPECT_FALSE((x_zero && y_zero) || (x_zero && z_zero) || (y_zero && z_zero)) <<
			"A face with <" << CGAL::to_double(o.dx()) << ", " << CGAL::to_double(o.dy()) << ", " << CGAL::to_double(o.dz()) << "> found no parallels";
	}
}

TEST_F(ComplicatedExtrudedElement, InitialHalfblockCountCorrect) {
	auto faces = e->geometry().oriented_faces(&c);
	auto rels = build_relations_grid(faces, &c);
	std::vector<block> dummy;

	std::list<oriented_area> x_bottom;
	std::list<oriented_area> x_lower_middle;
	std::list<oriented_area> x_left_lower;
	std::list<oriented_area> x_left_upper;
	std::list<oriented_area> x_top;

	std::list<oriented_area> y_left_bottom;
	std::list<oriented_area> y_left_middle;
	std::list<oriented_area> y_left_top;
	std::list<oriented_area> y_small_top;
	std::list<oriented_area> y_right_big;
	std::list<oriented_area> y_right_small;
	std::list<oriented_area> y_small_bottom;

	std::list<oriented_area> z;

	auto relevances_count = [&rels, &faces](size_t base_index) -> size_t { 
		size_t res = 0;
		for (size_t i = 0; i < faces.size(); ++i) {
			if (rels[base_index][i].contributes_to_envelope()) { ++res; }
		}
		return res;
	};

	auto degenerate = scan_for_degenerate_halfblocks(rels, faces.size(), *e, std::back_inserter(dummy));
	for (size_t i = 0; i < faces.size(); ++i) {
		if (degenerate.find(i) == degenerate.end()) {
			const oriented_area & this_base = faces[i];
			if (!CGAL::is_zero(this_base.orientation().dx())) {
				if (!this_base.sense() && this_base.height() == 0) {
					halfblocks_for_base(rels, i, &c, std::back_inserter(x_bottom));
				}
				else if (!this_base.sense() && this_base.height() == 2050) {
					EXPECT_EQ(1, relevances_count(i)) << "(horizontal lower middle)";
					halfblocks_for_base(rels, i, &c, std::back_inserter(x_lower_middle));
				}
				else if (this_base.sense() && this_base.height() == 8250) {
					EXPECT_EQ(1, relevances_count(i)) << "horizontal left lower)";
					halfblocks_for_base(rels, i, &c, std::back_inserter(x_left_lower));
				}
				else if (this_base.sense() && this_base.height() == 12120.109) {
					EXPECT_EQ(1, relevances_count(i)) << "(horizontal left uppe)r";
					halfblocks_for_base(rels, i, &c, std::back_inserter(x_left_upper));
				}
				else if (this_base.sense() && this_base.height() == 18195.109) {
					EXPECT_EQ(2, relevances_count(i)) << "(horizontal top)"; // diagonal is relevant because of the lack of projection intersection filtering
					halfblocks_for_base(rels, i, &c, std::back_inserter(x_top));
				}
				else {
					EXPECT_TRUE(false) << 
						"Unknown halfblock (horizontal) with height " << 
						CGAL::to_double(this_base.height());
				}
			}
			else if (!CGAL::is_zero(this_base.orientation().dy())) {
				if (this_base.sense() && this_base.height() == 0) {
					EXPECT_EQ(3, relevances_count(i)) << "(vertical left bottom)"; // diagonal is relevant because of the lack of projection intersection filtering
					halfblocks_for_base(rels, i, &c, std::back_inserter(y_left_bottom));
				}
				else if (this_base.sense() && this_base.height() == -2105) {
					EXPECT_EQ(2, relevances_count(i)) << "(vertical left middle)";
					halfblocks_for_base(rels, i, &c, std::back_inserter(y_left_middle));
				}
				else if (this_base.sense() && this_base.height() == -4050) {
					EXPECT_EQ(2, relevances_count(i)) << "(vertical left top)";
					halfblocks_for_base(rels, i, &c, std::back_inserter(y_left_top));
				}
				else if (!this_base.sense() && this_base.height() == -8200) {
					EXPECT_EQ(1, relevances_count(i)) << "(vertical small top)";
					halfblocks_for_base(rels, i, &c, std::back_inserter(y_small_top));
				}
				else if (!this_base.sense() && this_base.height() == -29200) {
					EXPECT_EQ(3, relevances_count(i)) << "(vertical right big)";
					halfblocks_for_base(rels, i, &c, std::back_inserter(y_right_big));
				}
				else if (this_base.sense() && this_base.height() == -28000) {
					EXPECT_EQ(2, relevances_count(i)) << "(vertical right small)"; // diagonal is relevant because of the lack of projection intersection filtering
					halfblocks_for_base(rels, i, &c, std::back_inserter(y_right_small));
				}
				else if (!this_base.sense() && this_base.height() == -8250) {
					EXPECT_EQ(1, relevances_count(i)) << "(vertical small bottom)";
					halfblocks_for_base(rels, i, &c, std::back_inserter(y_small_bottom));
				}
				else {
					EXPECT_TRUE(false) <<
						"Unknown halfblock (vertical) with height " <<
						CGAL::to_double(this_base.height());
				}
			}
			else if (!CGAL::is_zero(this_base.orientation().dz())) {
				EXPECT_EQ(1, relevances_count(i)) << "(z)";
				halfblocks_for_base(rels, i, &c, std::back_inserter(z));
			}
			else {
				EXPECT_TRUE(false) <<
					"Unknown halfblock (non-axis-aligned) with height " <<
					CGAL::to_double(this_base.height());
			}
		}
	}
	EXPECT_EQ(5, x_bottom.size());
	EXPECT_EQ(1, x_lower_middle.size());
	EXPECT_EQ(1, x_left_lower.size());
	EXPECT_EQ(1, x_left_upper.size());
	EXPECT_EQ(1, x_top.size());
	EXPECT_EQ(2, y_left_bottom.size());
	EXPECT_EQ(2, y_left_middle.size());
	EXPECT_EQ(2, y_left_top.size());
	EXPECT_EQ(1, y_small_top.size());
	EXPECT_EQ(3, y_right_big.size());
	EXPECT_EQ(1, y_right_small.size());
	EXPECT_EQ(1, y_small_bottom.size());
	EXPECT_EQ(2, z.size());
}

TEST_F(ComplicatedExtrudedElement, BlockCountCorrect) {
	EXPECT_EQ(15, build_blocks_for(*e, &c).size());
}

} // namespace

} // namespace impl

} // namespace blocking