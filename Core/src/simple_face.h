#pragma once

#include "precompiled.h"

#include "sbt-core.h"

class equality_context;

class simple_face {
private:
	typedef std::vector<point_3> loop;

	loop m_outer;
	std::vector<loop> m_inners; // should have same orientation as outer
	plane_3 m_plane;
	point_3 m_average_point;

	template <typename PointRange, typename VoidRange>
	simple_face(const PointRange outer, const VoidRange inners, const plane_3 & plane, const point_3 & point)
		: m_outer(outer.begin(), outer.end()),
		m_inners(inners.begin(), inners.end()),
		m_plane(plane),
		m_average_point(point)
	{ }

public:
	simple_face(const face & f, equality_context * c);
	simple_face(simple_face && src) { *this = std::move(src); }

	simple_face & operator = (simple_face && src);

	const loop & outer() const { return m_outer; }
	const std::vector<loop> & inners() const { return m_inners; }
	direction_3 orthogonal_direction() const { return m_plane.orthogonal_direction(); }
	const plane_3 & plane() const { return m_plane; }
	const point_3 & average_outer_point() const { return m_average_point; }

	simple_face reversed() const;

	std::vector<segment_3> all_edges_voids_reversed() const;

	simple_face transform(const transformation_3 & t) const;
};