#pragma once

#include "precompiled.h"

class equality_context;

namespace geometry_common {

bool is_valid(const polygon_2 & poly, double eps);

inline bool is_axis_aligned(const polygon_2 & poly) {
	for (auto edge = poly.edges_begin(); edge != poly.edges_end(); ++edge) {
		if (!(edge->is_vertical() || edge->is_horizontal())) {
			return false;
		}
	}
	return true;
}

inline bool share_sense(const direction_3 & a, const direction_3 & b) {
	return (a.to_vector() + b.to_vector()).squared_length() > a.to_vector().squared_length() + b.to_vector().squared_length();
}

template <class VecT>
inline VecT normalize(const VecT & v) {
	if (v.squared_length() < 1.0) {
		return (100 * v) / sqrt((100 * v).squared_length());
	}
	else {
		return v / sqrt(v.squared_length());
	}
}

template <class PointIter>
NT smallest_squared_distance(PointIter begin, PointIter end) {
	NT distance;
	bool got_first = false;
	for (auto p = begin; p != end; ++p) {
		for (auto q = p; q != end; ++q) {
			if (p != q) {
				NT this_dist = CGAL::square(p->x() - q->x()) + CGAL::square(p->y() - q->y());
				if (!got_first || this_dist < distance) {
					distance = this_dist;
					got_first = true;
				}
			}
		}
	}
	return distance;
}

NT regular_area(const polygon_2 & poly);

std::tuple<plane_3, point_3> calculate_plane_and_average_point(
	const std::vector<point_3> & loop,
	const equality_context & c);

} // namespace geometry_common