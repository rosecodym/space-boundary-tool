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
	// i'm not really sure what this test shows
	// but it sure shows something
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

	polyhedron_3 poly;
	builder b(false);
	poly.delegate(b);
	nef_polyhedron_3 nef(poly);
	EXPECT_EQ(4, nef.number_of_vertices());
	CGAL::Object_handle o = nef.locate(point_3(0.1, 0.1, 0.1));
	nef_polyhedron_3::Volume_const_handle v;
	ASSERT_TRUE(CGAL::assign(v, o));
	EXPECT_TRUE(v->mark());

	nef_polyhedron_3::Halffacet_const_handle f;
	o = nef.locate(point_3(0.25, 0, 0.25));
	ASSERT_TRUE(CGAL::assign(f, o));
	// here's where i would check the face normal, but i have no idea how CGAL
	// decides which halffacet to return

	// see, it looks like whatever the orientation of the faces, the nef 
	// polygon constructor will just "figure it out." this is with respect to 
	// the mark of the internal volume
	polyhedron_3 rev;
	b = builder(true);
	rev.delegate(b);
	nef = nef_polyhedron_3(rev);
	EXPECT_EQ(4, nef.number_of_vertices());
	o = nef.locate(point_3(0.1, 0.1, 0.1));
	ASSERT_TRUE(CGAL::assign(v, o));
	EXPECT_TRUE(v->mark());
	
	// it's *not* with respect to the orientation of the face normals though, 
	// so i have no idea what's going on
	o = nef.locate(point_3(0.25, 0, 0.25));
	ASSERT_TRUE(CGAL::assign(f, o));
	EXPECT_EQ(direction_3(0, 1, 0), f->plane().orthogonal_direction());
}

} // namespace