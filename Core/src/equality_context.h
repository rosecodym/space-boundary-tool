#pragma once

#include "precompiled.h"

#include "cgal-util.h"
#include "one_dimensional_equality_context.h"
#include "orientation.h"

class equality_context {
	
private:

	double tolerance;
	one_dimensional_equality_context xs_2d;
	one_dimensional_equality_context ys_2d;
	one_dimensional_equality_context xs_3d;
	one_dimensional_equality_context ys_3d;
	one_dimensional_equality_context zs_3d;
	one_dimensional_equality_context heights;
	std::vector<std::unique_ptr<orientation>> orientations;

	std::vector<std::pair<direction_3, vector_3>> directions;

	void init_constants();

	equality_context(const equality_context & src);
	equality_context & operator = (const equality_context & src);

public:

	equality_context(double tol) : tolerance(tol), heights(tol), xs_2d(tol), ys_2d(tol), xs_3d(tol), ys_3d(tol), zs_3d(tol) { init_constants(); }

	point_2 request_point(double x, double y) { return point_2(xs_2d.request(x), ys_2d.request(y)); }
	point_3 request_point(double x, double y, double z) { return point_3(xs_3d.request(x), ys_3d.request(y), zs_3d.request(z)); }
	NT request_height(double z) { return heights.request(z); }
	std::tuple<orientation *, bool> request_orientation(const direction_3 & d);

	static bool is_zero(double d, double eps) { return d < eps && d > -eps; }
	static bool is_zero(const NT & n, double eps) { return is_zero(CGAL::to_double(n), eps); }
	static bool are_equal(const NT & a, const NT & b, double eps) { return is_zero(a - b, eps); }
	static bool is_zero_squared(double d, double eps) { return d < eps * eps && d > -eps * eps; }
	static bool is_zero_squared(const NT & n, double eps) { return is_zero_squared(CGAL::to_double(n), eps); }

	template <class GeomT>
	static bool are_effectively_same(const GeomT & a, const GeomT & b, double eps) { return is_zero_squared(CGAL::squared_distance(a, b), eps); }
	template <>
	static bool are_effectively_same<point_3>(const point_3 & a, const point_3 & b, double eps) { return are_equal(a.x(), b.x(), eps) && are_equal(a.y(), b.y(), eps) && are_equal(a.z(), b.z(), eps); }
	static bool are_effectively_collinear(const point_2 & a, const point_2 & b, const point_2 & c, double eps) { return are_effectively_same(a, c, eps) || is_zero_squared(CGAL::squared_distance(b, line_2(a, c)), eps); }
	static bool are_effectively_collinear(const point_3 & a, const point_3 & b, const point_3 & c, double eps) { return are_effectively_same(a, c, eps) || is_zero_squared(CGAL::squared_distance(b, line_3(a, c)), eps); }
	static bool are_effectively_perpendicular(const vector_3 & a, const vector_3 & b, double eps) { return is_zero_squared((util::cgal::normalize(a) * util::cgal::normalize(b)), eps); }

	bool is_zero(double d) const { return is_zero(d, tolerance); }
	bool is_zero(const NT & n) const { return is_zero(CGAL::to_double(n)); }
	bool are_equal(const NT & a, const NT & b) const { return is_zero(a - b); }
	bool is_zero_squared(double d) const { return is_zero_squared(d, tolerance); }
	bool is_zero_squared(const NT & n) const { return is_zero_squared(CGAL::to_double(n), tolerance); }

	template <class GeomT>
	bool are_effectively_same(const GeomT & a, const GeomT & b) const { return is_zero_squared(CGAL::squared_distance(a, b)); }

	bool are_effectively_parallel(const vector_3 & a, const vector_3 & b) const { return is_zero_squared(CGAL::cross_product(util::cgal::normalize(a), util::cgal::normalize(b)).squared_length()); }
	bool are_effectively_parallel(const direction_3 & a, const direction_3 & b) const { return are_effectively_parallel(a.to_vector(), b.to_vector()); }
	bool are_effectively_collinear(const point_2 & a, const point_2 & b, const point_2 & c) const { return are_effectively_same(a, c) || is_zero_squared(CGAL::squared_distance(b, line_2(a, c))); }
	bool are_effectively_collinear(const point_3 & a, const point_3 & b, const point_3 & c) const { return are_effectively_same(a, c) || is_zero_squared(CGAL::squared_distance(b, line_3(a, c))); }
	bool are_effectively_perpendicular(const vector_3 & a, const vector_3 & b) const { return is_zero_squared((util::cgal::normalize(a) * util::cgal::normalize(b)), tolerance); }

	// one version or the other will be more convenient in different cases, so they're both provided
	point_2				snap(const point_2 & p) { return request_point(CGAL::to_double(p.x()), CGAL::to_double(p.y())); }
	point_3				snap(const point_3 & p) { return request_point(CGAL::to_double(p.x()), CGAL::to_double(p.y()), CGAL::to_double(p.z())); }
	direction_3			snap(const direction_3 & d);
	ray_3				snap(const ray_3 & r) { return ray_3(snap(r.point(0)), snap(r.direction())); }
	plane_3				snap(const plane_3 & pl) { return plane_3(point_3(0, 0, 0), snap(pl.orthogonal_direction())); }
	transformation_3 *	snap(transformation_3 * t) { return t; }
	polygon_2			snap(const polygon_2 & x);

	template <class T>
	void				snap(T * x) { *x = snap(*x); }

	NT					snap_height(const NT & z) { return request_height(CGAL::to_double(z)); }
	void				snap_height(NT * z) { *z = snap_height(*z); }
};