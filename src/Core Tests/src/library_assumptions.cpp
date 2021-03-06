#include "precompiled.h"

#include <gtest/gtest.h>

namespace {

TEST(LibraryAssumptions, BBox2Construction) {
	boost::optional<bbox_2> res;

	point_2 pts[] = {
		point_2(10, -300),
		point_2(8250, -300),
		point_2(8250, 10),
		point_2(10, 10)
	};

	ASSERT_FALSE(res);

	if (res) {
		res = *res + pts[0].bbox();
	}
	else {
		res = pts[0].bbox();
	}

	EXPECT_DOUBLE_EQ(10, res->xmin());
	EXPECT_DOUBLE_EQ(10, res->xmax());
	EXPECT_DOUBLE_EQ(-300, res->ymin());
	EXPECT_DOUBLE_EQ(-300, res->ymax());

	ASSERT_TRUE(res);

	if (res) {
		res = *res + pts[1].bbox();
	}
	else {
		res = pts[1].bbox();
	}

	EXPECT_DOUBLE_EQ(10, res->xmin());
	EXPECT_DOUBLE_EQ(8250, res->xmax());
	EXPECT_DOUBLE_EQ(-300, res->ymin());
	EXPECT_DOUBLE_EQ(-300, res->ymax());

	ASSERT_TRUE(res);

	if (res) {
		res = *res + pts[2].bbox();
	}
	else {
		res = pts[2].bbox();
	}

	EXPECT_DOUBLE_EQ(10, res->xmin());
	EXPECT_DOUBLE_EQ(8250, res->xmax());
	EXPECT_DOUBLE_EQ(-300, res->ymin());
	EXPECT_DOUBLE_EQ(10, res->ymax());

	ASSERT_TRUE(res);

	if (res) {
		res = *res + pts[3].bbox();
	}
	else {
		res = pts[3].bbox();
	}

	EXPECT_DOUBLE_EQ(10, res->xmin());
	EXPECT_DOUBLE_EQ(8250, res->xmax());
	EXPECT_DOUBLE_EQ(-300, res->ymin());
	EXPECT_DOUBLE_EQ(10, res->ymax());
}

TEST(LibraryAssumptions, Bbox2DoOverlap) {
	point_2 larger_pts[] = {
		point_2(0, -300),
		point_2(8250, -300),
		point_2(8250, 0),
		point_2(0, 0)
	};
	point_2 smaller_pts[] = {
		point_2(0, -300),
		point_2(4050, -300), 
		point_2(4050, 0),
		point_2(0, 0)
	};
	auto larger = larger_pts[0].bbox();
	auto smaller = smaller_pts[0].bbox();
	for (int i = 1; i < 4; ++i) {
		larger = larger + larger_pts[i].bbox();
		smaller = smaller + smaller_pts[i].bbox();
	}
	EXPECT_TRUE(CGAL::do_overlap(larger, smaller));
}

TEST(LibraryAssumptions, PointInHalfspace) {
	plane_3 pl(point_3(0, 0, 0), direction_3(0, 0, 1));
	EXPECT_TRUE(pl.has_on_positive_side(point_3(0, 0, 1)));
	EXPECT_FALSE(pl.has_on_positive_side(point_3(0, 0, 0)));
}		

const size_t ORIGIN = 0;
const size_t BACK = 1;
const size_t RIGHT = 2;
const size_t TOP = 3;

TEST(LibraryAssumptions, PolyhedronAssembly) {
	// This test is supposed to evaluate and test how polyhedron assembly in
	// CGAL works. The idea is this: assemble a polyhedron from some faces, and
	// check both the mark of the "internal" volume and the orientations of the
	// halffacets selected by placing a point directly on a facet. Then, reverse
	// the assembling faces and perform the same checks.

	// Unfortunately, there is some evidence that CGAL does not behave 
	// consistently! I have tested this on two computers. In both (flipped and
	// unflipped) cases, the enclosed volume has been marked, but in one case
	// the halffacet selected was oriented one way, and in the other case it
	// was the other way. This deserves a proper bug report to the CGAL mailing
	// list.
	class builder : public CGAL::Modifier_base<polyhedron_3::HDS> {             
	private:
		std::vector<point_3> points;
		std::vector<std::deque<size_t>> indices;
	public:
		builder(bool reversed) {
			points.resize(4);
			points[ORIGIN] = point_3(0, 0, 0);
			points[BACK] = point_3(0, 1, 0);
			points[RIGHT] = point_3(1, 0, 0);
			points[TOP] = point_3(0, 0, 1);
			indices.resize(4);
			indices[0].push_back(ORIGIN);
			indices[0].push_back(BACK);
			indices[0].push_back(RIGHT);
			indices[1].push_back(ORIGIN);
			indices[1].push_back(TOP);
			indices[1].push_back(BACK);
			indices[2].push_back(ORIGIN);
			indices[2].push_back(RIGHT);
			indices[2].push_back(TOP);
			indices[3].push_back(TOP);
			indices[3].push_back(RIGHT);
			indices[3].push_back(BACK);
			if (reversed) { boost::for_each(indices, [](std::deque<size_t> & face) { std::reverse(face.begin(), face.end()); }); }
		}
	
		void operator () (polyhedron_3::HDS & hds) {
			CGAL::Polyhedron_incremental_builder_3<polyhedron_3::HDS> b(hds, true);
			b.begin_surface(points.size(), indices.size());
			for (auto p = points.begin(); p != points.end(); ++p) {
				b.add_vertex(*p);
			}
			for (auto f = indices.begin(); f != indices.end(); ++f) {
				b.add_facet(f->begin(), f->end());
			}
			b.end_surface();
		}
	};

	// Assemble the polygon as normal (face normals out).
	polyhedron_3 poly;
	builder b(false);
	poly.delegate(b);
	nef_polyhedron_3 nef(poly);
	EXPECT_EQ(4, nef.number_of_vertices());
	CGAL::Object_handle o = nef.locate(point_3(0.1, 0.1, 0.1));
	nef_polyhedron_3::Volume_const_handle v;
	ASSERT_TRUE(CGAL::assign(v, o));
	// The enclosed volume appears to be marked.
	EXPECT_TRUE(v->mark());

	nef_polyhedron_3::Halffacet_const_handle f;
	o = nef.locate(point_3(0.25, 0, 0.25));
	ASSERT_TRUE(CGAL::assign(f, o));
	// Awhile ago I deleted the check for the halffacet normal because I wasn't
	// sure what it was supposed to be.

	// Now, construct the solid from its reversed faces (face normals in).
	polyhedron_3 rev;
	b = builder(true);
	rev.delegate(b);
	nef = nef_polyhedron_3(rev);
	EXPECT_EQ(4, nef.number_of_vertices());
	o = nef.locate(point_3(0.1, 0.1, 0.1));
	ASSERT_TRUE(CGAL::assign(v, o));
	// The enclosed volume is still marked.
	EXPECT_TRUE(v->mark());
	
	// On one machine I've checked, the selected halffacet points in. On 
	// another, it points out, and this test fails on the last predicate.
	o = nef.locate(point_3(0.25, 0, 0.25));
	ASSERT_TRUE(CGAL::assign(f, o));
	// Mystery predicate follows:
	EXPECT_EQ(direction_3(0, 1, 0), f->plane().orthogonal_direction());
}

TEST(LibraryAssumptions, NefPolygonSubtraction) {
	// This is an angled quadrilateral "cutting into" a square through the
	// square's upper-right corner. I understand that the numbers are obnoxious
	// but if I make them simpler then the problem doesn't become apparent.
	// What's going to happen here is that the "cut" will actually be slightly
	// "inside" the square, adding an extra vertex to the result.
	espoint_2 rect_pts[] = {
		espoint_2(7.0, 8.339190),
		espoint_2(8.769285, 8.339190),
		espoint_2(8.769285, 8.844385),
		espoint_2(7.0, 8.844385)
	};
	espoint_2 nonrect_pts[] = {
		espoint_2(8.665582, 8.744685),
		espoint_2(8.883634, 8.954685),
		espoint_2(8.883634, 11.0),
		espoint_2(8.665582, 11.0)
	};

	nef_polygon_2 rect(rect_pts, rect_pts + 4);
	nef_polygon_2 nonrect(nonrect_pts, nonrect_pts + 4);
	
	size_t v_count = 0;
	nef_polygon_2 diff = rect - nonrect;
	auto e = diff.explorer();
	for (auto v = e.vertices_begin(); v != e.vertices_end(); ++v) {
		if (e.is_standard(v)) { ++v_count; }
	}
	EXPECT_EQ(7, v_count);
}

TEST(LibraryAssumptions, NefPolygonSubtractionUpdate) {
	// This is an angled quadrilateral "cutting into" a square through the
	// square's upper-right corner. I understand that the numbers are obnoxious
	// but if I make them simpler then the problem doesn't become apparent.
	// What's going to happen here is that the "cut" will actually be slightly
	// "inside" the square, adding an extra vertex to the result.
	espoint_2 rect_pts[] = {
		espoint_2(7.0, 8.339190),
		espoint_2(8.769285, 8.339190),
		espoint_2(8.769285, 8.844385),
		espoint_2(7.0, 8.844385)
	};
	espoint_2 nonrect_pts[] = {
		espoint_2(8.665582, 8.744685),
		espoint_2(8.883634, 8.954685),
		espoint_2(8.883634, 11.0),
		espoint_2(8.665582, 11.0)
	};

	nef_polygon_2 rect(rect_pts, rect_pts + 4);
	nef_polygon_2 nonrect(nonrect_pts, nonrect_pts + 4);
	
	size_t v_count = 0;
	rect -= nonrect;
	auto e = rect.explorer();
	for (auto v = e.vertices_begin(); v != e.vertices_end(); ++v) {
		if (e.is_standard(v)) { ++v_count; }
	}
	EXPECT_EQ(7, v_count);
}

} // namespace