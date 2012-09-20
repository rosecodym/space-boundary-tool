#include "precompiled.h"

#include "flatten.h"

namespace geometry_common {

using geometry_2d::polygon_with_holes_2;

point_2 flatten(const point_2 & p) {
	return point_2(CGAL::to_double(p.x()), CGAL::to_double(p.y()));
}

polygon_2 flatten(const polygon_2 & poly) {
	auto flattened = boost::make_iterator_range(poly.vertices_begin(), poly.vertices_end())
		| boost::adaptors::transformed([](const point_2 & p) { return flatten(p); });
	return polygon_2(flattened.begin(), flattened.end());
}

polygon_with_holes_2 flatten(const polygon_with_holes_2 & pwh) {
	return polygon_with_holes_2(flatten(pwh.outer()), pwh.holes() | boost::adaptors::transformed([](const polygon_2 & poly) { return flatten(poly); }));
}

} // namespace geometry_common