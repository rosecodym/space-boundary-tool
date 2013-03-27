#include "precompiled.h"

#include "exceptions.h"
#include "geometry_common.h"
#include "poly_builder.h"
#include "simple_face.h"
#include "stringification.h"

#include "solid_geometry_util.h"

namespace solid_geometry {

namespace impl {

namespace {

std::vector<simple_face> reconcile_orientations(
	const std::vector<simple_face> & all_faces, 
	const std::vector<int> & group_memberships,
	int group,
	const std::vector<std::vector<face_relationship>> & relationships)
{
	size_t root_ix = 0;
	face_status root_status = OK;

	std::vector<face_status> statuses(all_faces.size(), NOT_DECIDED);

	std::queue<std::tuple<size_t, face_status>> process_queue;
	process_queue.push(std::make_tuple(root_ix, root_status));

	while (!process_queue.empty()) {
		size_t next_ix;
		face_status next_status;
		std::tie(next_ix, next_status) = process_queue.front();
		process_queue.pop();
		if (statuses[next_ix] == NOT_DECIDED) {
			statuses[next_ix] = next_status;
			for (size_t i = 0; i < relationships.size(); ++i) {
				if (statuses[i] == NOT_DECIDED && relationships[next_ix][i] != NOT_CONNECTED) {
					process_queue.push(std::make_tuple(i, (face_status)(relationships[next_ix][i] * next_status)));
				}
			}
		}
	}

	std::vector<simple_face> res;
	for (size_t i = 0; i < all_faces.size(); ++i) {
		if (group_memberships[i] == group) {
			res.push_back(statuses[i] == FLIP ? all_faces[i].reversed() : all_faces[i]);
		}
	}

	return res;
}

std::vector<std::vector<simple_face>> to_volume_groups(std::vector<simple_face> && all_faces) {
	typedef size_t face_ix_t;

	struct segment_comparator : public std::unary_function<segment_3, bool> {
		bool operator () (const segment_3 & a, const segment_3 & b) const {
			return a.source() == b.source() ? a.target() < b.target() : a.source() < b.source();
		}
	};

	std::vector<std::vector<face_relationship>> relationships(all_faces.size(), std::vector<face_relationship>(all_faces.size(), NOT_CONNECTED));

	std::map<segment_3, face_ix_t, segment_comparator> edge_memberships;

	boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS> graph;

	for (size_t face_ix = 0; face_ix < all_faces.size(); ++face_ix) {
		boost::for_each(all_faces[face_ix].all_edges_voids_reversed(), [&edge_memberships, &relationships, &graph, face_ix](const segment_3 & e) {
			bool opposite = false;
			auto exists = edge_memberships.find(e);
			if (exists == edge_memberships.end()) {
				exists = edge_memberships.find(e.opposite());
				opposite = true;
			}
			if (exists != edge_memberships.end()) {
				size_t match_ix = exists->second;
				relationships[face_ix][match_ix] = relationships[match_ix][face_ix] = opposite ? MATCH : MISMATCH;
				boost::add_edge(face_ix, match_ix, graph);
			}
			else {
				edge_memberships.insert(std::make_pair(e, face_ix));
			}
		});
	}
	
	std::vector<int> group_memberships(all_faces.size());
	int group_count = boost::connected_components(graph, &group_memberships[0]);

	std::vector<std::vector<simple_face>> res;
	for (int i = 0; i < group_count; ++i) {
		res.push_back(reconcile_orientations(all_faces, group_memberships, i, relationships));
	}

	return res;
}

void merge_adjacent_faces(polyhedron_3 * poly) {
	using geometry_common::calculate_plane_and_average_point;

	for (auto f = poly->facets_begin(); f != poly->facets_end(); ++f) {
		std::vector<point_3> points;
		auto v = f->facet_begin();
		auto end = v;
		CGAL_For_all(v, end) {
			points.push_back(v->vertex()->point());
		}
		f->plane() = std::get<0>(calculate_plane_and_average_point(points));
	}

find_redundant_edges:
	for (auto f = poly->facets_begin(); f != poly->facets_end(); ++f) {
		auto g = f->facet_begin();
		auto end = g;
		CGAL_For_all(g, end) {
			if (!g->is_border_edge() &&
				g->opposite()->facet()->plane() == f->plane())
			{
				poly->join_facet(g);
				goto find_redundant_edges;
			}
		}
	}

	poly->keep_largest_connected_components(1);

	// CGAL offers some hole removal facilities but I can't get them to work.
find_holes:
	for (auto h = poly->halfedges_begin(); h != poly->halfedges_end(); ++h) {
		if (h->is_border() &&
			h->vertex_degree() >= 3 &&
			h->opposite()->vertex_degree() >= 3)
		{
			poly->join_facet(h->opposite());
			goto find_holes;
		}
	}
}

} // namespace

nef_polyhedron_3 extrusion_to_nef(const extrusion_information & ext, equality_context * c) {
	const simple_face & f = std::get<0>(ext);
	const vector_3 & extrusion = std::get<1>(ext);
	
	transformation_3 extrude(CGAL::TRANSLATION, extrusion);
	polyhedron_3 poly;
	poly_builder builder(f.outer(), extrude, c);
	poly.delegate(builder);
	if (!poly.is_valid() || !poly.is_closed()) {
		return nef_polyhedron_3();
	}
	nef_polyhedron_3 res(poly);
	boost::for_each(f.inners(), [&res, &extrude, c](const std::vector<point_3> & inner) {
		polyhedron_3 poly;
		poly_builder builder(inner, extrude, c);
		poly.delegate(builder);
		res -= nef_polyhedron_3(poly);
	});
	return res;
}

std::vector<simple_face> faces_from_brep(const brep & b, equality_context * c) {
	std::vector<simple_face> res;
	for (size_t i = 0; i < b.face_count; ++i) {
		try {
			res.push_back(simple_face(b.faces[i], c));
		}
		catch (invalid_face_exception & /*ex*/) {
			throw bad_brep_exception();
		}
	}
	return res;
}

nef_polyhedron_3 simple_faces_to_nef(std::vector<simple_face> && all_faces) {
	// There will be no reported problems if at least one of the face groups is 
	// "good". Whether this is a bug or feature I leave as an exercise to the
	// reader - remember that "Hey it kind of works!" is kind of the status quo
	// in this domain.
	auto as_groups = to_volume_groups(std::move(all_faces));
	nef_polyhedron_3 res;
	boost::for_each(as_groups, [&res](const std::vector<simple_face> & group) {
		res += volume_group_to_nef(group);
	});
	return res.interior();
}

nef_polyhedron_3 volume_group_to_nef(const std::vector<simple_face> & group) {
	polyhedron_3 poly;
	poly_builder b(group);
	poly.delegate(b);
	merge_adjacent_faces(&poly);
	if (!poly.is_closed()) {
		// Why not throw an exception here? Well, currently, this only ever
		// happens with a bad brep, so presumably a bad_brep_exception would do
		// the job nicely. Unfortunately I can't really guarantee this will
		// happen in the future - this function could later be used in some 
		// other case. So we're going to treat an empty return as an error code
		// and let upstream handle it.
		return nef_polyhedron_3::EMPTY;
	}
	else {
		return nef_polyhedron_3(poly).interior();
	}
}

} // namespace impl

} // namespace solid_geometry