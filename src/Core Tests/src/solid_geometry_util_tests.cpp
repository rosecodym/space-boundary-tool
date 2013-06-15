#include "precompiled.h"

#include <gtest/gtest.h>

#include "common.h"
#include "equality_context.h"
#include "sbt-core.h"
#include "simple_face.h"
#include "solid_geometry_util.h"

namespace solid_geometry {

namespace impl {

namespace {

TEST(FacesFromBrep, VoidsAreFiltered) {
	// A cube with a column cut out of the middle of it.
	equality_context c(0.01);
	point lln;
	lln.x = lln.y = lln.z = 0.0;
	point lrn = lln;
	lrn.x = 3.0;
	point urn = lrn;
	urn.z = 3.0;
	point uln = urn;
	uln.x = 0.0;
	point ulf = uln;
	ulf.y = 3.0;
	point urf = ulf;
	urf.x = 3.0;
	point lrf = urf;
	lrf.z = 0.0;
	point llf = lrf;
	llf.x = 0.0;
	point illf = llf;
	llf.x = 1.0;
	llf.y = 2.0;
	point illn = illf;
	illn.y = 1.0;
	point ilrn = illn;
	ilrn.x = 2.0;
	point ilrf = ilrn;
	ilrf.y = 2.0;
	point iurf = ilrf;
	iurf.z = 3.0;
	point iurn = iurf;
	iurn.y = 1.0;
	point iuln = iurn;
	iuln.x = 1.0;
	point iulf = iuln;
	iuln.y = 2.0;
	auto set_for_voids = [](face * f) {
		f->outer_boundary.vertex_count = 4;
		f->outer_boundary.vertices = (point *)malloc(4 * sizeof(point));
		f->void_count = 1;
		f->voids = (polyloop *)malloc(sizeof(polyloop));
		f->voids[0].vertex_count = 4;
		f->voids[0].vertices = (point *)malloc(4 * sizeof(point));
	};
	auto set_no_voids = [](face * f) {
		f->outer_boundary.vertex_count = 4;
		f->outer_boundary.vertices = (point *)malloc(4 * sizeof(point));
		f->void_count = 0;
		f->voids = nullptr;
	};
	brep b;
	b.face_count = 10;
	b.faces = (face *)malloc(b.face_count * sizeof(face));
	set_no_voids(&b.faces[0]);
	b.faces[0].outer_boundary.vertices[0] = lln;
	b.faces[0].outer_boundary.vertices[1] = uln;
	b.faces[0].outer_boundary.vertices[2] = ulf;
	b.faces[0].outer_boundary.vertices[3] = llf;
	set_no_voids(&b.faces[1]);
	b.faces[1].outer_boundary.vertices[0] = llf;
	b.faces[1].outer_boundary.vertices[1] = ulf;
	b.faces[1].outer_boundary.vertices[2] = urf;
	b.faces[1].outer_boundary.vertices[3] = lrf;
	set_no_voids(&b.faces[2]);
	b.faces[2].outer_boundary.vertices[0] = lrf;
	b.faces[2].outer_boundary.vertices[1] = urf;
	b.faces[2].outer_boundary.vertices[2] = urn;
	b.faces[2].outer_boundary.vertices[3] = lrn;
	set_no_voids(&b.faces[3]);
	b.faces[3].outer_boundary.vertices[0] = lrn;
	b.faces[3].outer_boundary.vertices[1] = urn;
	b.faces[3].outer_boundary.vertices[2] = uln;
	b.faces[3].outer_boundary.vertices[3] = lln;
	set_for_voids(&b.faces[4]);
	b.faces[4].outer_boundary.vertices[0] = lln;
	b.faces[4].outer_boundary.vertices[1] = llf;
	b.faces[4].outer_boundary.vertices[2] = lrf;
	b.faces[4].outer_boundary.vertices[3] = lrn;
	b.faces[4].voids[0].vertices[0] = illn;
	b.faces[4].voids[0].vertices[1] = illf;
	b.faces[4].voids[0].vertices[2] = ilrf;
	b.faces[4].voids[0].vertices[3] = ilrn;
	set_for_voids(&b.faces[5]);
	b.faces[5].outer_boundary.vertices[0] = uln;
	b.faces[5].outer_boundary.vertices[1] = ulf;
	b.faces[5].outer_boundary.vertices[2] = urf;
	b.faces[5].outer_boundary.vertices[3] = urn;
	b.faces[5].voids[0].vertices[0] = iuln;
	b.faces[5].voids[0].vertices[1] = iulf;
	b.faces[5].voids[0].vertices[2] = iurf;
	b.faces[5].voids[0].vertices[3] = iurn;
	set_no_voids(&b.faces[6]);
	b.faces[6].outer_boundary.vertices[0] = illn;
	b.faces[6].outer_boundary.vertices[1] = illf;
	b.faces[6].outer_boundary.vertices[2] = iulf;
	b.faces[6].outer_boundary.vertices[3] = iuln;
	set_no_voids(&b.faces[7]);
	b.faces[7].outer_boundary.vertices[0] = illf;
	b.faces[7].outer_boundary.vertices[1] = iulf;
	b.faces[7].outer_boundary.vertices[2] = iurf;
	b.faces[7].outer_boundary.vertices[3] = illf;
	set_no_voids(&b.faces[8]);
	b.faces[8].outer_boundary.vertices[0] = ilrf;
	b.faces[8].outer_boundary.vertices[1] = iurf;
	b.faces[8].outer_boundary.vertices[2] = iurn;
	b.faces[8].outer_boundary.vertices[3] = ilrn;
	set_no_voids(&b.faces[9]);
	b.faces[9].outer_boundary.vertices[0] = ilrn;
	b.faces[9].outer_boundary.vertices[1] = illn;
	b.faces[9].outer_boundary.vertices[2] = iuln;
	b.faces[9].outer_boundary.vertices[3] = iurn;

	auto faces = faces_from_brep(b, &c);
	EXPECT_EQ(6, faces.size());
	for (auto f = faces.begin(); f != faces.end(); ++f) {
		EXPECT_TRUE(f->voids().empty());
	}
}

TEST(VolumeGroupToNef, ArtificialFaceSplitsDoNotCauseEmptiness) {
	// This tests a stunt that Revit's been pulling recently. Imagine a
	// tetrahedral "tent" with one of the faces split like "flaps." If there's
	// no corresponding vertex at the split on the "floor" face then CGAL will
	// consider the resulting polyhedron open. (In addition, even when this
	// splitting happens to work out space faces get all messed up.)

	equality_context c(0.01);

	std::vector<simple_face> faces;

	faces.push_back(simple_face(
		create_face(3,
			simple_point(-1, 0, 0),
			simple_point(0, 1, 0),
			simple_point(1, 0, 0)), false, &c));
	faces.push_back(simple_face(
		create_face(3,
			simple_point(-1, 0, 0),
			simple_point(0, 0, 1),
			simple_point(0, 1, 0)), false, &c));
	faces.push_back(simple_face(
		create_face(3,
			simple_point(0, 0, 1),
			simple_point(1, 0, 0),
			simple_point(0, 1, 0)), false, &c));
	faces.push_back(simple_face(
		create_face(3,
			simple_point(-1, 0, 0),
			simple_point(0, 0, 0),
			simple_point(0, 0, 1)), false, &c));
	faces.push_back(simple_face(
		create_face(3,
			simple_point(0, 0, 0),
			simple_point(1, 0, 0),
			simple_point(0, 0, 1)), false, &c));

	nef_polyhedron_3 nef = volume_group_to_nef(std::move(faces), c);
	EXPECT_FALSE(nef.is_empty());
}

// Legacy tests follow

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

TEST(SolidGeometryUtil, ExtrusionToNefSimple) {
	equality_context c(0.01);

	face f = create_face(4,
		simple_point(1, 15, 0),
		simple_point(10, 15, 0),
		simple_point(10, 2, 0),
		simple_point(1, 2, 0));

	nef_polyhedron_3 nef = 
		extrusion_to_nef(std::make_tuple(
			simple_face(f, false, &c), 
			vector_3(0, 0, 300)), &c);

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

TEST(SolidGeometryUtil, ExtrusionToNefReversedBase) {
	equality_context c(0.01);

	face f = create_face(4,
		simple_point(1, 2, 0),
		simple_point(10, 2, 0),
		simple_point(10, 15, 0),
		simple_point(1, 15, 0));

	nef_polyhedron_3 nef = 
		extrusion_to_nef(std::make_tuple(
			simple_face(f, false, &c), 
			vector_3(0, 0, 300)), &c);

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