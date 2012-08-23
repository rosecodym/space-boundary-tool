#include "precompiled.h"

#include <gtest/gtest.h>

#include "equality_context.h"
#include "sbt-core-helpers.h"
#include "simple_face.h"
#include "solid_conversion_operations.h"

namespace solid_geometry {

namespace impl {

namespace {

std::string stringify(const point_3 & p) {
	std::stringstream ss;
	ss << "<" << CGAL::to_double(p.x()) << ", " << CGAL::to_double(p.y()) << ", " << CGAL::to_double(p.z()) << ">";
	return ss.str();
}


std::string stringify(const plane_3 & pl) {
	std::stringstream ss;
	ss << CGAL::to_double(pl.a()) << "x + " << CGAL::to_double(pl.b()) << "y + " << CGAL::to_double(pl.c()) << "z + " << CGAL::to_double(pl.d()) << " = 0";
	return ss.str();
}

std::string stringify(const direction_3 & d) {
	std::stringstream ss;
	ss << "<" << CGAL::to_double(d.dx()) << ", " << CGAL::to_double(d.dy()) << ", " << CGAL::to_double(d.dz()) << ">";
	return ss.str();
}

struct face_information {
	bool * found_flag;
	direction_3 normal;
	point_3 example_point;
	face_information() { }
	face_information(bool * flag, const direction_3 & d, const point_3 & p)
		: found_flag(flag), normal(d), example_point(p)
	{ }
};

TEST(SolidConversion, SimpleExtrusion) {
	equality_context c(0.01);
	face f;
	f.void_count = 0;
	f.voids = nullptr;
	f.outer_boundary.vertex_count = 4;
	f.outer_boundary.vertices = (point *)malloc(sizeof(point) * f.outer_boundary.vertex_count);
	polyloop * outer = get_outer_boundary_handle(&f);
	set_vertex(outer, 3, 1, 2, 0);
	set_vertex(outer, 2, 10, 2, 0);
	set_vertex(outer, 1, 10, 15, 0);
	set_vertex(outer, 0, 1, 15, 0);

	nef_polyhedron_3 nef = extrusion_to_nef(std::make_tuple(simple_face(f, &c), vector_3(0, 0, 300)), &c);

	EXPECT_EQ(2, nef.number_of_volumes());

	int marked_halffacets = 0;
	nef_halffacet_handle h;
	CGAL_forall_halffacets(h, nef) {
		if (h->mark()) { 
			EXPECT_EQ(1, std::distance(h->facet_cycles_begin(), h->facet_cycles_end()));
			++marked_halffacets; 
		}
	}
	EXPECT_EQ(12, marked_halffacets);

	int marked_volumes = 0;
	nef_volume_handle v;
	nef_volume_handle marked_v;
	CGAL_forall_volumes(v, nef) {
		if (v->mark()) { 
			EXPECT_EQ(1, std::distance(v->shells_begin(), v->shells_end()));
			++marked_volumes; 
			marked_v = v;
		}
	}
	ASSERT_EQ(1, marked_volumes);

	class face_checker {
	private:
		bool low;
		bool high;
		bool left;
		bool right;
		bool front;
		bool back;
		std::map<NT, face_information> locations;
	public:
		face_checker() : low(0), high(0), left(0), right(0), front(0), back(0) { 
			locations[0] = face_information(&low, direction_3(0, 0, -1), point_3(5, 5, 0));
			locations[300] = face_information(&high, direction_3(0, 0, 1), point_3(5, 5, 300));
			locations[1] = face_information(&left, direction_3(-1, 0, 0), point_3(1, 8, 50));
			locations[10] = face_information(&right, direction_3(1, 0, 0), point_3(10, 8, 50));
			locations[2] = face_information(&front, direction_3(0, -1, 0), point_3(4, 2, 100));
			locations[15] = face_information(&back, direction_3(0, 1, 0), point_3(4, 15, 100));
		}
		void visit(nef_vertex_handle /*h*/) { }
		void visit(nef_halfedge_handle /*h*/) { }
		void visit(nef_halffacet_handle h) { 
			plane_3 pl = h->plane().opposite();
			auto match = locations.find(CGAL::sqrt(CGAL::squared_distance(pl, point_3(0, 0, 0))));
			ASSERT_NE(match, locations.end());
			EXPECT_FALSE(*match->second.found_flag);
			*match->second.found_flag = true;
			EXPECT_EQ(match->second.normal, pl.orthogonal_direction());
			EXPECT_TRUE(pl.has_on(match->second.example_point)) << "Didn't have point " << stringify(match->second.example_point);
		}
		void visit(nef_shalfedge_handle /*h*/) { }
		void visit(nef_shalfloop_handle /*h*/) { }
		void visit(nef_sface_handle /*h*/) { }
	};

	nef_polyhedron_3::Shell_entry_const_iterator sit;
	face_checker checker;
	nef.visit_shell_objects(nef_sface_handle(marked_v->shells_begin()), checker);
}

TEST(SolidConversion, ReversedBaseExtrusion) {
	equality_context c(0.01);
	face f;
	f.void_count = 0;
	f.voids = nullptr;
	f.outer_boundary.vertex_count = 4;
	f.outer_boundary.vertices = (point *)malloc(sizeof(point) * f.outer_boundary.vertex_count);
	polyloop * outer = get_outer_boundary_handle(&f);
	set_vertex(outer, 0, 1, 2, 0);
	set_vertex(outer, 1, 10, 2, 0);
	set_vertex(outer, 2, 10, 15, 0);
	set_vertex(outer, 3, 1, 15, 0);

	nef_polyhedron_3 nef = extrusion_to_nef(std::make_tuple(simple_face(f, &c), vector_3(0, 0, 300)), &c);

	EXPECT_EQ(2, nef.number_of_volumes());

	int marked_halffacets = 0;
	nef_halffacet_handle h;
	CGAL_forall_halffacets(h, nef) {
		if (h->mark()) { 
			EXPECT_EQ(1, std::distance(h->facet_cycles_begin(), h->facet_cycles_end()));
			++marked_halffacets; 
		}
	}
	EXPECT_EQ(12, marked_halffacets);

	int marked_volumes = 0;
	nef_volume_handle v;
	nef_volume_handle marked_v;
	CGAL_forall_volumes(v, nef) {
		if (v->mark()) { 
			EXPECT_EQ(1, std::distance(v->shells_begin(), v->shells_end()));
			++marked_volumes; 
			marked_v = v;
		}
	}
	ASSERT_EQ(1, marked_volumes);

	class face_checker {
	private:
		bool low;
		bool high;
		bool left;
		bool right;
		bool front;
		bool back;
		std::map<NT, face_information> locations;
	public:
		face_checker() : low(0), high(0), left(0), right(0), front(0), back(0) { 
			locations[0] = face_information(&low, direction_3(0, 0, -1), point_3(5, 5, 0));
			locations[300] = face_information(&high, direction_3(0, 0, 1), point_3(5, 5, 300));
			locations[1] = face_information(&left, direction_3(-1, 0, 0), point_3(1, 8, 50));
			locations[10] = face_information(&right, direction_3(1, 0, 0), point_3(10, 8, 50));
			locations[2] = face_information(&front, direction_3(0, -1, 0), point_3(4, 2, 100));
			locations[15] = face_information(&back, direction_3(0, 1, 0), point_3(4, 15, 100));
		}
		void visit(nef_vertex_handle /*h*/) { }
		void visit(nef_halfedge_handle /*h*/) { }
		void visit(nef_halffacet_handle h) { 
			plane_3 pl = h->plane().opposite();
			auto match = locations.find(CGAL::sqrt(CGAL::squared_distance(pl, point_3(0, 0, 0))));
			ASSERT_NE(match, locations.end());
			EXPECT_FALSE(*match->second.found_flag);
			*match->second.found_flag = true;
			EXPECT_EQ(match->second.normal, pl.orthogonal_direction());
			EXPECT_TRUE(pl.has_on(match->second.example_point)) << "Didn't have point " << stringify(match->second.example_point);
		}
		void visit(nef_shalfedge_handle /*h*/) { }
		void visit(nef_shalfloop_handle /*h*/) { }
		void visit(nef_sface_handle /*h*/) { }
	};

	nef_polyhedron_3::Shell_entry_const_iterator sit;
	face_checker checker;
	nef.visit_shell_objects(nef_sface_handle(marked_v->shells_begin()), checker);
}

} // namespace

} // namespace impl

} // namespace solid_geometry