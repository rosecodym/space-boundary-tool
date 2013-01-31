#pragma once

#include "precompiled.h"

#include "one_dimensional_equality_context.h"

template <typename K>
class number_collection {
	typedef typename K::FT			NT;
	typedef typename K::Point_2		point_2;
	typedef typename K::Point_3		point_3;
	typedef typename K::Direction_3	direction_3;
	typedef typename K::Vector_3	vector_3;

private:
	double tolerance;
	one_dimensional_equality_context<NT> xs_2d;
	one_dimensional_equality_context<NT> ys_2d;
	one_dimensional_equality_context<NT> xs_3d;
	one_dimensional_equality_context<NT> ys_3d;
	one_dimensional_equality_context<NT> zs_3d;
	one_dimensional_equality_context<NT> heights;

	std::vector<direction_3> directions;

	void init_constants()
	{
		heights.request(0.0); heights.request(1.0);
		xs_2d.request(0.0); xs_2d.request(1.0);
		ys_2d.request(0.0); ys_2d.request(1.0);
		xs_3d.request(0.0); xs_3d.request(1.0);
		ys_3d.request(0.0); ys_3d.request(1.0);
		zs_3d.request(0.0); zs_3d.request(1.0);
	}

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

	static bool are_effectively_parallel(
		const direction_3 & a, 
		const direction_3 & b, 
		double eps) 
	{
		vector_3 v_a = a.to_vector();
		assert(!CGAL::is_zero(v_a.squared_length()));
		vector_3 v_b = b.to_vector();
		assert(!CGAL::is_zero(v_b.squared_length()));
		auto denominator = v_a.squared_length() * v_b.squared_length();
		// The whole point of this stupid exact geometry stuff is that this
		// assert will *never* trip. But let's just make sure.
		assert(!CGAL::is_zero(denominator));
		return one_dimensional_equality_context<NT>::is_zero_squared(
			CGAL::cross_product(v_a, v_b).squared_length() / denominator,
			eps);
	}

};