#include "precompiled.h"

#include "number_collection.h"

#include "geometry_common.h"

transformation_3 build_flatten(const direction_3 & d) {
	// use a quaternion

	NT ZERO(0.0);
	NT ONE(1.0);

	auto dir = normalize(d.vector());

	auto zhat = vector_3(ZERO, ZERO, ONE);
	if (dir == -zhat) {
		dir = vector_3(ZERO, ZERO, ZERO - ONE - ONE); // something arbitrary
	}
	auto v = normalize(dir + zhat);

	auto qv = CGAL::cross_product(v, zhat);
	auto qw = v * zhat;

	auto nq = qw * qw + qv.squared_length();
	assert(nq > ZERO);
	auto s = 2 / nq;

	auto w = qw;
	auto x = qv.x();
	auto y = qv.y();
	auto z = qv.z();

	return transformation_3(
		ONE-(y*y*s+z*z*s),	x*y*s-w*z*s,		x*z*s+w*y*s,
		x*y*s+w*z*s,		ONE-(x*x*s+z*z*s),	y*z*s-w*x*s,
		x*z*s-w*y*s,		y*z*s+w*x*s,		ONE-(x*x*s+y*y*s));
}