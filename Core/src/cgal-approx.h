#pragma once

#include "precompiled.h"

typedef double									approx_NT;
typedef CGAL::Simple_cartesian<approx_NT>		approx_K;
typedef CGAL::Point_3<approx_K>					approx_point_3;
typedef CGAL::Direction_3<approx_K>				approx_direction_3;
typedef CGAL::Vector_3<approx_K>				approx_vector_3;
typedef CGAL::Line_3<approx_K>					approx_line_3;
typedef CGAL::Plane_3<approx_K>					approx_plane_3;
typedef CGAL::Aff_transformation_3<approx_K>	approx_transformation_3;

namespace util {

namespace approx {

inline approx_point_3 convert_point(const point_3 & p) {
	return approx_point_3(CGAL::to_double(p.x()), CGAL::to_double(p.y()), CGAL::to_double(p.z()));
}

inline approx_line_3 convert_line(const line_3 & line) {
	return approx_line_3(convert_point(line.point(0)), convert_point(line.point(1)));
}

inline approx_plane_3 convert_plane(const plane_3 & pl) {
	return approx_plane_3(CGAL::to_double(pl.a()), CGAL::to_double(pl.b()), CGAL::to_double(pl.c()), CGAL::to_double(pl.d()));
}

} // namespace approx

} // namespace util