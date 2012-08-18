#pragma once

#include "precompiled.h"

class equality_context;

namespace util {

namespace cgal {

template <class VecT>
inline VecT normalize(const VecT & v) {
	if (v.squared_length() < 1.0) {
		return (100 * v) / sqrt((100 * v).squared_length());
	}
	else {
		return v / sqrt(v.squared_length());
	}
}

bool polygon_has_no_adjacent_duplicates(const polygon_2 & p, equality_context * c);

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

bbox_3 nef_bounding_box(const nef_polyhedron_3 & nef);

inline bool share_sense(const direction_3 & a, const direction_3 & b) {
	return (a.to_vector() + b.to_vector()).squared_length() > a.to_vector().squared_length() + b.to_vector().squared_length();
}

} // namespace cgal

} // namespace util