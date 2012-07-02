#include "precompiled.h"

#include "equality_context.h"
#include "printing-macros.h"
#include "sbt-core.h"

#include "simple_face.h"

namespace {

typedef CGAL::Simple_cartesian<double> iK;
typedef CGAL::Point_3<iK> ipoint_3;
typedef CGAL::Line_3<iK> iline_3;

typedef std::vector<point_3> loop;

bool are_same(const ipoint_3 & a, const ipoint_3 & b, const equality_context & ctxt) {
	return ctxt.is_zero(a.x() - b.x()) && ctxt.is_zero(a.y() - b.y()) && ctxt.is_zero(a.z() - b.z());
}

bool are_collinear(const ipoint_3 & a, const ipoint_3 & b, const ipoint_3 & c, const equality_context & ctxt) {
	return are_same(a, c, ctxt) || ctxt.is_zero_squared(CGAL::squared_distance(b, iline_3(a, c)));
}

std::vector<ipoint_3> cleanup_polyloop(const polyloop & p, const equality_context & c) {
	std::vector<ipoint_3> res;

	for (size_t i = 0; i < p.vertex_count; ++i) {
		ipoint_3 pt(p.vertices[i].x, p.vertices[i].y, p.vertices[i].z);

		if (res.empty()) {
			res.push_back(pt);
		}

		else if (!are_same(pt, res.front(), c) && !are_same(pt, res.back(), c)) {
			if (res.size() > 1 && are_collinear(res[res.size() - 2], res.back(), pt, c)) {
				res.pop_back();
			}
			res.push_back(pt);
		}
	}

	return res;
}

loop create_loop(const polyloop & p, equality_context * c) {
	auto clean_points = cleanup_polyloop(p, *c);
	loop res;
	boost::transform(clean_points, std::back_inserter(res), [c](const ipoint_3 & p) { return c->request_point(p.x(), p.y(), p.z()); });
	return res;
}

std::tuple<plane_3, point_3> calculate_plane_and_average_point(const loop & l) {
	// http://cs.haifa.ac.il/~gordon/plane.pdf
	NT a(0.0);
	NT b(0.0);
	NT c(0.0);
	NT x(0.0);
	NT y(0.0);
	NT z(0.0);
	for (size_t i = 0; i < l.size(); ++i) {
		const point_3 & curr = l[i];
		const point_3 & next = l[(i+1) % l.size()];
		a += (curr.y() - next.y()) * (curr.z() + next.z());
		b += (curr.z() - next.z()) * (curr.x() + next.x());
		c += (curr.x() - next.x()) * (curr.y() + next.y());
		x += curr.x();
		y += curr.y();
		z += curr.z();
	}
	vector_3 avg_vec(x / l.size(), y / l.size(), z / l.size());
	return std::make_tuple(plane_3(a, b, c, -avg_vec * vector_3(a, b, c)), CGAL::ORIGIN + avg_vec);
}

bool opposite_sense(const direction_3 & a, const direction_3 & b) {
	vector_3 vec_a = a.to_vector();
	vector_3 vec_b = b.to_vector();
	return (vec_a + vec_b).squared_length() < vec_a.squared_length() + vec_b.squared_length();
}

} // namespace

simple_face::simple_face(const face & f, equality_context * c) : m_outer(create_loop(f.outer_boundary, c)) {
	std::tie(m_plane, m_average_point) = calculate_plane_and_average_point(m_outer);
	std::transform(f.voids, f.voids + f.void_count, std::back_inserter(m_inners), [c, this](const polyloop & p) -> loop {
		loop inner = create_loop(p, c);
		if (opposite_sense(m_plane.orthogonal_direction(), std::get<0>(calculate_plane_and_average_point(inner)).orthogonal_direction())) {
			boost::reverse(inner);
		}
		return inner;
	});
}

simple_face & simple_face::operator = (simple_face && src) {
	if (&src != this) {
		m_outer = std::move(src.m_outer); 
		m_inners = std::move(src.m_inners); 
		m_plane = std::move(src.m_plane);
		m_average_point = std::move(src.m_average_point);
	} 
	return *this;
}

simple_face simple_face::reversed() const {
	return simple_face(
		m_outer | boost::adaptors::reversed,
		m_inners | boost::adaptors::transformed([](const std::vector<point_3> & inner) { return std::vector<point_3>(inner.rbegin(), inner.rend()); }),
		m_plane.opposite(),
		m_average_point);
}

std::vector<segment_3> simple_face::all_edges_voids_reversed() const {
	std::vector<segment_3> res;
	for (size_t i = 0; i < m_outer.size(); ++i) {
		res.push_back(segment_3(m_outer[i], m_outer[(i+1) % m_outer.size()]));
	}
	res.push_back(segment_3(m_outer.back(), m_outer.front()));
	boost::for_each(m_inners, [&res](const std::vector<point_3> & inner) {
		for (size_t i = 0; i < inner.size(); ++i) {
			res.push_back(segment_3(inner[(i+1) % inner.size()], inner[i]));
		}
		res.push_back(segment_3(inner.front(), inner.back()));
	});
	return res;
}