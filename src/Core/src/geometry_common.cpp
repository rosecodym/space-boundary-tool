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

std::tuple<plane_3, point_3> calculate_plane_and_average_point(
	const std::vector<point_3> & loop,
	const equality_context & ctxt) 
{
	// http://cs.haifa.ac.il/~gordon/plane.pdf
	int pcount = int(loop.size());
	NT a(0.0);
	NT b(0.0);
	NT c(0.0);
	NT x(0.0);
	NT y(0.0);
	NT z(0.0);
	for (size_t i = 0; i < pcount; ++i) {
		const point_3 & curr = loop[i];
		const point_3 & next = loop[(i+1) % pcount];
		a += (curr.y() - next.y()) * (curr.z() + next.z());
		b += (curr.z() - next.z()) * (curr.x() + next.x());
		c += (curr.x() - next.x()) * (curr.y() + next.y());
		x += curr.x();
		y += curr.y();
		z += curr.z();
	}
	vector_3 avg_vec(x / pcount, y / pcount, z / pcount);
	a = ctxt.is_zero(a) ? 0.0 : CGAL::to_double(a);
	b = ctxt.is_zero(b) ? 0.0 : CGAL::to_double(b);
	c = ctxt.is_zero(c) ? 0.0 : CGAL::to_double(c);
	NT d = -avg_vec * vector_3(a, b, c);
	return std::make_tuple(plane_3(a, b, c, d), CGAL::ORIGIN + avg_vec);
}

} // namespace geometry_common