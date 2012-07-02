#include "precompiled.h"

#include "../../Core/src/sbt-core.h"

#include "equality_context.h"
#include "exceptions.h"

#include "slice.h"

namespace {
	
int marked_volume_count(const nef_polyhedron_3 & nef) {
	int c = 0;
	nef_polyhedron_3::Volume_const_handle v;
	CGAL_forall_volumes(v, nef) {
		if (v->mark()) { ++c; }
	}
	return c;
}

int halffacet_with_marked_volume_count(const nef_polyhedron_3 & nef) {
	int c = 0;
	nef_halffacet_handle h;
	CGAL_forall_halffacets(h, nef) {
		if (h->incident_volume()->mark()) {
			++c;
		}
	}
	return c;
}

point_3 get_exact_point(const point & p, equality_context * c) {
	return point_3(c->request_coordinate(p.x), c->request_coordinate(p.y), c->request_coordinate(p.z));
}

std::deque<point_3> load_polyloop(const polyloop & p, equality_context * c) {
	std::deque<point_3> results;

	size_t p_ix;

	auto to_exact = [c](const point & p) { return get_exact_point(p, c); };

	results.push_back(to_exact(p.vertices[0]));

	for (p_ix = 1; p_ix < p.vertex_count; ++p_ix) {
		results.push_back(to_exact(p.vertices[p_ix]));
		if (results.back() != results.front()) {
			break;
		}
		results.pop_back();
	}

	if (p_ix == p.vertex_count) {
		throw polyloop_clean_failure_exception(polyloop_clean_failure_exception::NO_INITIAL);
	}

	point_3 next_point;
	++p_ix;
	for (; p_ix < p.vertex_count; ++p_ix) {
		next_point = to_exact(p.vertices[p_ix]);
#ifndef NDEBUG
		point second_to_last;
		second_to_last.x = to_double(results[results.size() - 2].x());
		second_to_last.y = to_double(results[results.size() - 2].y());
		second_to_last.z = to_double(results[results.size() - 2].z());
		point last;
		last.x = to_double(results[results.size() - 1].x());
		last.y = to_double(results[results.size() - 1].y());
		last.z = to_double(results[results.size() - 1].z());
		point next;
		next.x = to_double(next_point.x());
		next.y = to_double(next_point.y());
		next.z = to_double(next_point.z());
#endif
		if (c->are_effectively_collinear(results[results.size() - 2], results[results.size() - 1], next_point)) {
			results.back() = next_point;
		}
		else {
			results.push_back(next_point);
		}
	}

	if (results.size() < 3) {
		throw polyloop_clean_failure_exception(polyloop_clean_failure_exception::CLEANED_TO_TWO);
	}

	if (c->are_effectively_collinear(results[results.size() - 2], results[results.size() - 1], results[0])) {
		results.pop_back();
		if (results.size() < 3) {
			throw polyloop_clean_failure_exception(polyloop_clean_failure_exception::CLEANED_TO_TWO);
		}
	}
	auto first = results.begin();
	if (c->are_effectively_collinear(results[results.size() - 1], results[0], results[1])) {
		results.pop_front();
		if (results.size() < 3) {
			throw polyloop_clean_failure_exception(polyloop_clean_failure_exception::CLEANED_TO_TWO);
		}
	}

	return results;
}

class c_to_poly_converter : public CGAL::Modifier_base<polyhedron_3::HDS> {
private:
	std::vector<point_3> points;
	std::vector<std::deque<size_t>> indices;

public:
	c_to_poly_converter(const brep & b, equality_context * c) {
		for (size_t i = 0; i < b.face_count; ++i) {
			indices.push_back(std::deque<size_t>());
			std::deque<point_3> this_face = load_polyloop(b.faces[i].outer_boundary, c);
			boost::for_each(this_face, [this](const point_3 & p) {
				size_t k;
				for (k = 0; k < points.size(); ++k) {
					if (points[k] == p) {
						indices.back().push_back(k);
						break;
					}
				}
				if (k == points.size()) {
					points.push_back(p);
					indices.back().push_back(k);
				}
			});
		}
	}

	c_to_poly_converter(const extruded_area_solid & ext, equality_context * c) {
		if (ext.area.void_count != 0) {
			throw bad_input_exception(bad_input_exception::SOLID_WITH_VOIDS);
		}

		transformation_3 extrude(CGAL::TRANSLATION, vector_3(
			c->is_zero(ext.ext_dx) ? 0 : ext.ext_dx,
			c->is_zero(ext.ext_dy) ? 0 : ext.ext_dy,
			c->is_zero(ext.ext_dz) ? 0 : ext.ext_dz) * ext.extrusion_depth);

		std::deque<point_3> base = load_polyloop(ext.area.outer_boundary, c);
		size_t base_point_count = base.size();

		boost::copy(base, std::back_inserter(points));
		boost::transform(base, std::back_inserter(points), [&extrude](const point_3 & p) { return extrude(p); });
		indices.resize(base_point_count + 2);
		for (size_t i = 0; i < base_point_count; ++i) {
			indices[0].push_back(i);
			indices[1].push_front(i + base_point_count);
			indices[i + 2].push_back(i + base_point_count);
			indices[i + 2].push_back(i);
			indices[(i + 1) % base_point_count + 2].push_front(i + base_point_count);
			indices[(i + 1) % base_point_count + 2].push_front(i);
		}
	}

	void operator () (polyhedron_3::HDS & hds) {
		CGAL::Polyhedron_incremental_builder_3<polyhedron_3::HDS> b(hds, true);
		b.begin_surface(points.size(), indices.size());
		boost::for_each(points, [&b](const point_3 & p) { b.add_vertex(p); });
		boost::for_each(indices, [&b](const std::deque<size_t> & is) { b.add_facet(is.begin(), is.end()); });
		b.end_surface();
	}
};

nef_polyhedron_3 convert_to_nef(const solid & s, equality_context * c) {
	polyhedron_3 poly;
	poly.delegate(s.rep_type == REP_BREP ? c_to_poly_converter(s.rep.as_brep, c) : c_to_poly_converter(s.rep.as_ext, c));
	return nef_polyhedron_3(poly);
}

solid convert_to_solid(const nef_polyhedron_3 & nef) {
	solid s;
	s.rep_type = REP_BREP;
	brep & b = s.rep.as_brep;
	b.face_count = halffacet_with_marked_volume_count(nef);
	assert(b.face_count != 0);
	b.faces = (face *)malloc(sizeof(face) * b.face_count);

	nef_halffacet_handle hfacet;
	size_t i = 0;
	CGAL_forall_halffacets(hfacet, nef) {
		if (hfacet->incident_volume()->mark()) {
			auto cycle = hfacet->facet_cycles_begin();
			nef_polyhedron_3::SHalfedge_around_facet_const_circulator start(cycle);
			nef_polyhedron_3::SHalfedge_around_facet_const_circulator end = start;
			std::vector<point_3> loop;
			CGAL_For_all(start, end) {
				loop.push_back(start->source()->center_vertex()->point());
			}
			b.faces[i].outer_boundary.vertex_count = loop.size();
			b.faces[i].outer_boundary.vertices = (point *)malloc(sizeof(point) * b.faces[i].outer_boundary.vertex_count);
			for (size_t j = 0; j < loop.size(); ++j) {
				b.faces[i].outer_boundary.vertices[j].x = to_double(loop[j].x());
				b.faces[i].outer_boundary.vertices[j].y = to_double(loop[j].y());
				b.faces[i].outer_boundary.vertices[j].z = to_double(loop[j].z());
			}
			b.faces[i].void_count = 0;
			b.faces[i].voids = nullptr;
			++i;
		}
	}
	assert(i == b.face_count);

	return s;
}

bool are_oriented_similarly(const vector_3 & a, const vector_3 & b) {
	return
		sign(a.x()) == sign(b.x()) &&
		sign(a.y()) == sign(b.y()) &&
		sign(a.z()) == sign(b.z());
}

vector_3 normalize(const vector_3 & v) {
	double dx = to_double(v.x());
	double dy = to_double(v.y());
	double dz = to_double(v.z());
	double mag = sqrt(dx * dx + dy * dy + dz * dz);
	return v / mag;
}

plane_3 find_base_plane(const nef_polyhedron_3 & nef, const vector_3 & dir, equality_context * c) {
	std::vector<plane_3> candidates;
	nef_halffacet_handle hfacet;
	CGAL_forall_halffacets(hfacet, nef) {
		if (hfacet->incident_volume()->mark() && c->are_effectively_parallel(normalize(hfacet->plane().orthogonal_vector()), dir) && are_oriented_similarly(hfacet->plane().orthogonal_vector(), dir)) {
			candidates.push_back(hfacet->plane());
		}
	}
	// "inefficient" but the number of cases when more than two will be compared is miniscule
	for (auto candidate = candidates.begin(); candidate != candidates.end(); ++candidate) {
		ray_3 test_ray(candidate->point(), candidate->orthogonal_direction());
		bool ok = true;
		for (auto check_against = candidates.begin(); check_against != candidates.end(); ++check_against) {
			if (check_against != candidate && !CGAL::do_intersect(test_ray, *check_against)) {
				ok = false;
				break;
			}
		}
		if (ok) {
			return *candidate;
		}
	}
}

} // namespace

void slice(const solid & s, const vector_3 & vec, double thicknesses[], int layer_count, solid results[], equality_context * c)
{
	nef_polyhedron_3 whole = convert_to_nef(s, c);
	int vc;
	assert((vc = marked_volume_count(whole)) == 1);

	plane_3 start = find_base_plane(whole, vec, c);
	plane_3 end;

	std::vector<nef_polyhedron_3> nefs;

	for (int i = 0; i < layer_count - 1; ++i) {
		end = start.transform(transformation_3(CGAL::TRANSLATION, vec * thicknesses[i]));
		nefs.push_back(whole - nef_polyhedron_3(start) - nef_polyhedron_3(end.opposite()));
		assert((vc = marked_volume_count(nefs.back())) == 1);
		start = end;
	}
	nefs.push_back(whole - nef_polyhedron_3(start));

	for (int i = 0; i < layer_count; ++i) {
		results[i] = convert_to_solid(nefs[i]);
	}
}