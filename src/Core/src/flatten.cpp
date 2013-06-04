#include "precompiled.h"

#include "flatten.h"

#include "polygon_with_holes_2.h"

namespace geometry_common {

using geometry_2d::polygon_with_holes_2;

point_2 flatten(const point_2 & p) {
	return point_2(CGAL::to_double(p.x()), CGAL::to_double(p.y()));
}

polygon_2 flatten(const polygon_2 & poly) {
	std::vector<point_2> flattened;
	for (auto p = poly.vertices_begin(); p != poly.vertices_end(); ++p) {
		flattened.push_back(flatten(*p));
	}
	return polygon_2(flattened.begin(), flattened.end());
}

polygon_with_holes_2 flatten(const polygon_with_holes_2 & pwh) {
	using boost::transform;
	std::vector<polygon_2> holes;
	transform(pwh.holes(), std::back_inserter(holes), [](const polygon_2 & h) {
		return flatten(h);
	});
	return polygon_with_holes_2(flatten(pwh.outer()), holes);
}

} // namespace geometry_common