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

} // namespace geometry_common