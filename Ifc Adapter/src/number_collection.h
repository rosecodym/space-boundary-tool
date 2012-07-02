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

	void init_constants();

	number_collection(const number_collection & disabled);
	number_collection & operator = (const number_collection & disabled);

public:

	number_collection(double tol) : tolerance(tol), heights(tol), xs_2d(tol), ys_2d(tol), xs_3d(tol), ys_3d(tol), zs_3d(tol) { init_constants(); }

	point_2 request_point(double x, double y) { return point_2(xs_2d.request(x), ys_2d.request(y)); }
	point_3 request_point(double x, double y, double z) { return point_3(xs_3d.request(x), ys_3d.request(y), zs_3d.request(z)); }
	NT request_height(double z) { return heights.request(z); }

	direction_3 request_direction(double dx, double dy, double dz) { 
		point_3 p = request_point(dx, dy, dz);
		return direction_3(p.x(), p.y(), p.z());
	}

};