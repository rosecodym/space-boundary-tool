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
		assert(res.back().size() >= 3);
	}

	return res;
}

void merge_adjacent_faces(polyhedron_3 * poly, const equality_context & c) {
	using geometry_common::calculate_plane_and_average_point;

	for (auto f = poly->facets_begin(); f != poly->facets_end(); ++f) {
		std::vector<point_3> points;
		auto v = f->facet_begin();
		auto end = v;
		CGAL_For_all(v, end) {
			points.push_back(v->vertex()->point());
		}
		f->plane() = std::get<0>(calculate_plane_and_average_point(points, c));
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

std::vector<oriented_area> extrusion_to_faces(
	const extrusion_information & ext,
	equality_context * c)
{
	simple_face ext_base = std::get<0>(ext);
	vector_3 ext_vec = std::get<1>(ext);
	direction_3 ebdir = ext_base.orthogonal_direction();
	direction_3 evdir = ext_vec.direction();
	bool senses_opposite = !geometry_common::share_sense(ebdir, evdir);

	simple_face base = senses_opposite ? ext_base : ext_base.reversed();
	transformation_3 extrude(CGAL::TRANSLATION, std::get<1>(ext));

	auto create_sides = [&extrude, c](const std::vector<point_3> & base) 
		-> std::vector<oriented_area> 
	{
		typedef std::deque<point_3> res_loop;
		std::vector<point_3> target;
		boost::transform(base, std::back_inserter(target), extrude);
		std::vector<res_loop> res_loops(base.size());
		for (size_t i = 0; i < base.size(); ++i) {
			res_loops[i].push_back(target[i]);
			res_loops[i].push_back(base[i]);
			res_loops[(i + 1) % base.size()].push_front(target[i]);
			res_loops[(i + 1) % base.size()].push_front(base[i]);
		}
		std::vector<oriented_area> res;
		for (auto loop = res_loops.begin(); loop != res_loops.end(); ++loop) {
			auto as_vec = std::vector<point_3>(loop->begin(), loop->end());
			res.push_back(oriented_area(as_vec, c));
		}
		return res;
	};

	std::vector<oriented_area> res;
	res.push_back(oriented_area(base, c));
	res.push_back(oriented_area(base.reversed().transformed(extrude), c));
	boost::copy(create_sides(base.outer()), std::back_inserter(res));

	for (auto h = base.inners().begin(); h != base.inners().end(); ++h) {
		std::vector<point_3> reversed(h->rbegin(), h->rend());
		boost::copy(create_sides(reversed), std::back_inserter(res));
	}

	return res;
}

nef_polyhedron_3 extrusion_to_nef(const extrusion_information & ext, equality_context * c) {
	const simple_face & f = std::get<0>(ext);
	const vector_3 & extrusion = std::get<1>(ext);

	assert(f.is_planar());
	
	transformation_3 extrude(CGAL::TRANSLATION, extrusion);
	polyhedron_3 poly;
	auto builder = poly_builder::create(f.outer(), extrude);
	poly.delegate(builder);
	if (!poly.is_valid() || !poly.is_closed()) {
		return nef_polyhedron_3();
	}
	nef_polyhedron_3 res(poly);
	boost::for_each(f.inners(), [&res, &extrude, c](const std::vector<point_3> & inner) {
		polyhedron_3 poly;
		auto builder = poly_builder::create(inner, extrude);
		poly.delegate(builder);
		res -= nef_polyhedron_3(poly);
	});
	return res;
}

std::vector<simple_face> faces_from_brep(const brep & b, equality_context * c) {
	std::vector<simple_face> res;
	for (size_t i = 0; i < b.face_count; ++i) {
		try {
			res.push_back(simple_face(b.faces[i], false, c));
		}
		catch (invalid_face_exception & /*ex*/) {
			throw bad_brep_exception();
		}
	}
	return res;
}

nef_polyhedron_3 simple_faces_to_nef(
	std::vector<simple_face> && all_faces,
	const equality_context & c)
{
	// There will be no reported problems if at least one of the face groups is 
	// "good". Whether this is a bug or feature I leave as an exercise to the
	// reader - remember that "Hey it kind of works!" is kind of the status quo
	// in this domain.
	auto as_groups = to_volume_groups(std::move(all_faces));
	nef_polyhedron_3 res;
	boost::for_each(as_groups, [&](const std::vector<simple_face> & group) {
		res += volume_group_to_nef(group, c);
	});
	return res.interior();
}

nef_polyhedron_3 volume_group_to_nef(
	const std::vector<simple_face> & group,
	const equality_context & c)
{
	polyhedron_3 poly;
	auto b = poly_builder::create(group);
	poly.delegate(b);
	merge_adjacent_faces(&poly, c);
	if (!poly.is_valid() || !poly.is_closed()) {
		return nef_polyhedron_3::EMPTY;
	}
	else {
		return nef_polyhedron_3(poly).interior();
	}
}

} // namespace impl

} // namespace solid_geometry