#include "precompiled.h"

#include "approximated_curve.h"

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