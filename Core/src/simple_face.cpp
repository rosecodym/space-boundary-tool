#include "precompiled.h"

#include "cleanup_loop.h"
#include "equality_context.h"
#include "exceptions.h"
#include "sbt-core.h"

#include "simple_face.h"

namespace {

typedef std::vector<point_3> loop;

loop create_loop(const polyloop & p, equality_context * c) {
	std::vector<ipoint_3> pts;
	for (size_t i = 0; i < p.vertex_count; ++i) {
		pts.push_back(ipoint_3(p.vertices[i].x, p.vertices[i].y, p.vertices[i].z));
	}
	loop res;
	if (geometry_common::cleanup_loop(&pts, c->height_epsilon())) {
		boost::transform(pts, std::back_inserter(res), [c](const ipoint_3 & p) { return c->request_point(p.x(), p.y(), p.z()); });
	}
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
	vector_3 avg_vec(x / int(l.size()), y / int(l.size()), z / int(l.size()));
	return std::make_tuple(plane_3(a, b, c, -avg_vec * vector_3(a, b, c)), CGAL::ORIGIN + avg_vec);
}

bool opposite_sense(const direction_3 & a, const direction_3 & b) {
	vector_3 vec_a = a.to_vector();
	vector_3 vec_b = b.to_vector();
	return (vec_a + vec_b).squared_length() < vec_a.squared_length() + vec_b.squared_length();
}

} // namespace

simple_face::simple_face(const face & f, equality_context * c) : m_outer(create_loop(f.outer_boundary, c)) {
	if (m_outer.size() < 3) {
		throw invalid_face_exception();
	}
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
	typedef std::vector<point_3> loop;
	std::vector<loop> inners;
	boost::transform(
		m_inners, 
		std::back_inserter(inners),
		[](const loop & inner) { return loop(inner.rbegin(), inner.rend()); });
	return simple_face(
		m_outer | boost::adaptors::reversed,
		inners,
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