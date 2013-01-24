#include "precompiled.h"

#include "equality_context.h"

#include "geometry_common.h"

namespace geometry_common {

bool is_valid(const polygon_2 & poly, double eps) {
	for (auto e = poly.edges_begin(); e != poly.edges_end(); ++e) {
		if (equality_context::is_zero_squared(e->squared_length(), eps)) {
			return false;
		}
	}
	return true;
}

NT regular_area(const polygon_2 & poly) {
	// http://www.mathopenref.com/coordpolygonarea.html
	NT res = 0.0;
	for (size_t i = 0; i < poly.size(); ++i) {
		const point_2 & curr = poly[i];
		const point_2 & next = poly[(i + 1) % poly.size()];
		res += curr.x() * next.y() - curr.y() * next.x();
	}
	return res / 2;
}

} // namespace geometry_common