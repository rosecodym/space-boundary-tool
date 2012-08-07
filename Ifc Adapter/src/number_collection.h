#pragma once

#include "precompiled.h"

#include "cgal-typedefs.h"

#include "one_dimensional_equality_context.h"

class number_collection {

private:
	double tolerance;
	one_dimensional_equality_context xs_2d;
	one_dimensional_equality_context ys_2d;
	one_dimensional_equality_context xs_3d;
	one_dimensional_equality_context ys_3d;
	one_dimensional_equality_context zs_3d;
	one_dimensional_equality_context heights;

	std::vector<direction_3> directions;

	void init_constants();

	static bool share_sense(const direction_3 & a, const direction_3 & b) {
		return (a.to_vector() + b.to_vector()).squared_length() > a.to_vector().squared_length() + b.to_vector().squared_length();
	}

	number_collection(const number_collection & disabled);
	number_collection & operator = (const number_collection & disabled);

public:

	number_collection(double tol) : tolerance(tol), heights(tol), xs_2d(tol), ys_2d(tol), xs_3d(tol), ys_3d(tol), zs_3d(tol) { init_constants(); }

	point_2 request_point(double x, double y) { return point_2(xs_2d.request(x), ys_2d.request(y)); }
	point_3 request_point(double x, double y, double z) { return point_3(xs_3d.request(x), ys_3d.request(y), zs_3d.request(z)); }
	NT request_height(double z) { return heights.request(z); }

	direction_3 request_direction(double dx, double dy, double dz) {
		direction_3 requested(dx, dy, dz);
		auto exists = boost::find_if(directions, [&requested, this](const direction_3 & d) {
			return are_effectively_parallel(d, requested, tolerance);
		});
		if (exists != directions.end()) {
			return share_sense(requested, *exists) ? *exists : -*exists;
		}
		else {
			directions.push_back(requested);
			return directions.back();
		}
	}

	static bool are_effectively_parallel(const direction_3 & a, const direction_3 & b, double eps) {
		vector_3 v_a = a.to_vector();
		v_a = v_a / CGAL::sqrt(v_a.squared_length());
		vector_3 v_b = b.to_vector();
		v_b = v_b / CGAL::sqrt(v_b.squared_length());
		return one_dimensional_equality_context::is_zero_squared(CGAL::cross_product(v_a, v_b).squared_length(), eps);
	}

};