#include "precompiled.h"

#include "cgal-util.h"
#include "equality_context.h"

#include "orientation.h"

namespace {

transformation_3 build_flatten(const direction_3 & d) {
	// use a quaternion

	NT ZERO(0.0);
	NT ONE(1.0);

	auto dir = util::cgal::normalize(d.vector());

	auto zhat = vector_3(ZERO, ZERO, ONE);
	if (dir == -zhat) {
		dir = vector_3(ZERO, ZERO, ZERO - ONE - ONE); // something arbitrary
	}
	auto v = util::cgal::normalize(dir + zhat);

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

} // namespace

orientation::orientation(const direction_3 & d)
	: m_direction(d),
	m_flatten(build_flatten(d)),
	m_unflatten(m_flatten.inverse())
{ }

std::string orientation::to_string() const {
	char buf[128];
	sprintf(buf, "<%f, %f, %f>", CGAL::to_double(dx()), CGAL::to_double(dy()), CGAL::to_double(dz()));
	return std::string(buf);
}

bool orientation::are_perpendicular(const orientation & a, const orientation & b, double eps) {
	if (eps != 0.0 && eps != -0.0) {
		return equality_context::is_zero_squared((a.vector().squared_length() + b.vector().squared_length() - (a.vector() - b.vector()).squared_length()), eps);
	}
	else {
		return CGAL::is_zero(a.m_direction.to_vector() * b.m_direction.to_vector());
	}
}