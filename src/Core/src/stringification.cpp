#include "precompiled.h"

#include "stringification.h"

namespace reporting {

std::string to_string(const point_2 & p) {
	char buf[128];
	sprintf(buf, "(%f, %f)", CGAL::to_double(p.x()), CGAL::to_double(p.y()));
	return buf;
}

std::string to_string(const point_3 & p) {
	char buf[128];
	sprintf(buf, "(%f, %f, %f)", CGAL::to_double(p.x()), CGAL::to_double(p.y()), CGAL::to_double(p.z()));
	return buf;
}

std::string to_string(const direction_3 & d) {
	char buf[128];
	sprintf(buf, "<%f, %f, %f>", CGAL::to_double(d.dx()), CGAL::to_double(d.dy()), CGAL::to_double(d.dz()));
	return buf;
}

std::string to_string(const vector_3 & v) {
	char buf[128];
	sprintf(buf, "<%f, %f, %f>", CGAL::to_double(v.x()), CGAL::to_double(v.y()), CGAL::to_double(v.z()));
	return buf;
}

std::string to_string(const polygon_2 & poly) {
	return to_string(boost::make_iterator_range(poly.vertices_begin(), poly.vertices_end()));
}

} // namespace reporting