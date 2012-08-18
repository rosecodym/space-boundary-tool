#pragma once

#include "precompiled.h"

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

} // namespace geometry_common