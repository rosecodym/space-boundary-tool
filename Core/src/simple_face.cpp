#include "precompiled.h"

#include "cleanup_loop.h"
#include "equality_context.h"
#include "exceptions.h"
#include "sbt-core.h"
#include "stringification.h"

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

} // namespace

simple_face::simple_face(
	const face & f, 
	bool force_planar, 
	equality_context * c) 
{
	using boost::transform;
	using geometry_common::calculate_plane_and_average_point;
	using geometry_common::share_sense;

	auto raw_loop = create_loop(f.outer_boundary, c);
	if (raw_loop.size() < 3) { throw invalid_face_exception(); }

	plane_3 raw_pl;
	std::tie(raw_pl, m_average_point) = 
		calculate_plane_and_average_point(raw_loop);

	auto snapped_dir = c->snap(raw_pl.orthogonal_direction());
	m_plane = plane_3(m_average_point, snapped_dir);

	auto project = [this](const point_3 & p) { return m_plane.projection(p); };

	if (force_planar) {
		transform(raw_loop, std::back_inserter(m_outer), project);
	}
	else {
		m_outer = raw_loop;
	}

	for (size_t i = 0; i < f.void_count; ++i) {
		auto inner = create_loop(f.voids[i], c);
		auto inner_pl = std::get<0>(calculate_plane_and_average_point(inner));
		auto inner_dir = inner_pl.orthogonal_direction();
		if (!share_sense(m_plane.orthogonal_direction(), inner_dir)) {
			boost::reverse(inner);
		}
		if (force_planar) {
			m_inners.push_back(loop());
			transform(inner, std::back_inserter(m_inners.back()), project);
		}
		else {
			m_inners.push_back(inner);
		}
	}

#ifndef NDEBUG
	direction_3 normal = m_plane.orthogonal_direction();
	debug_dx = CGAL::to_double(normal.dx());
	debug_dy = CGAL::to_double(normal.dy());
	debug_dz = CGAL::to_double(normal.dz());
#endif

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

bool simple_face::is_planar() const {
	for (auto p = m_outer.begin(); p != m_outer.end(); ++p) {
		if (!m_plane.has_on(*p)) { return false; }
	}
	for (auto hole = m_inners.begin(); hole != m_inners.end(); ++hole) {
		for (auto p = hole->begin(); p != hole->end(); ++p) {
			if (!m_plane.has_on(*p)) { return false; }
		}
	}
	return true;
}

std::string simple_face::to_string() const {
	std::stringstream ss;
	ss << "Outer:\n";
	for (auto p = m_outer.begin(); p != m_outer.end(); ++p) {
		ss << reporting::to_string(*p) << std::endl;
	}
	for (auto hole = m_inners.begin(); hole != m_inners.end(); ++hole) {
		ss << "Hole:\n";
		for (auto p = hole->begin(); p != hole->end(); ++p) {
			ss << reporting::to_string(*p) << std::endl;
		}
	}
	return ss.str();
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

simple_face simple_face::transformed(const transformation_3 & t) const {
	loop new_outer;
	boost::transform(m_outer, std::back_inserter(new_outer), t);
	std::vector<loop> new_inners;
	for (auto h = m_inners.begin(); h != m_inners.end(); ++h) {
		new_inners.push_back(loop());
		boost::transform(*h, std::back_inserter(new_inners.back()), t);
	}
	return simple_face(new_outer, new_inners, t(m_plane), t(m_average_point));
}