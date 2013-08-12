#include "precompiled.h"

#include "approximated_curve.h"

approximated_curve::approximated_curve(
	const point_3 & p1, 
	const point_3 & p2,
	const direction_3 & normal,
	double area_on_left,
	double length_ratio)
	: p1_(p1),
	  p2_(p2),
	  n_(normal),
	  length_ratio_(length_ratio),
	  area_(area_on_left)
{ 
#ifndef NDEBUG
	dp1_[0] = CGAL::to_double(p1.x());
	dp1_[1] = CGAL::to_double(p1.y());
	dp1_[2] = CGAL::to_double(p1.z());
	dp2_[0] = CGAL::to_double(p2.x());
	dp2_[1] = CGAL::to_double(p2.y());
	dp2_[2] = CGAL::to_double(p2.z());
#endif
}

bool approximated_curve::matches(
	double x1, double y1, double z1,
	double x2, double y2, double z2,
	double eps) const
{
	// I am well aware that this process could be made vastly more efficient

	auto close = [eps](const point_3 & p, double x, double y, double z) {
		return
			abs(CGAL::to_double(p.x()) - x) <= eps &&
			abs(CGAL::to_double(p.y()) - y) <= eps &&
			abs(CGAL::to_double(p.z()) - z) <= eps;
	};

	return 
		(close(p1_, x1, y1, z1) && close(p2_, x2, y2, z2)) ||
		(close(p2_, x1, y1, z1) && close(p1_, x2, y2, z2));
}