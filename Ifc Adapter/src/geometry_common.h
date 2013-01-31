#pragma once

#include "precompiled.h"

#include "number_collection.h"
#include "sbt-ifcadapter.h"

template <typename KernelT>
CGAL::Vector_3<KernelT> normalize(const CGAL::Vector_3<KernelT> & v) {
	return v / CGAL::sqrt(v.squared_length());
}

inline transformation_3 build_translation(const direction_3 & dir, NT depth) {
	return transformation_3(CGAL::TRANSLATION, normalize(dir.vector()) * depth);
}

template <typename KernelT>
CGAL::Point_3<KernelT> from_c_point(point p) {
	return CGAL::Point_3<KernelT>(p.x, p.y, p.z);
}

inline point to_c_point(const point_3 & p) {
	point pt;
	pt.x = CGAL::to_double(p.x());
	pt.y = CGAL::to_double(p.y());
	pt.z = CGAL::to_double(p.z());
	return pt;
}

inline bool operator == (const point & lhs, const point & rhs) {
	return
		lhs.x == rhs.x &&
		lhs.y == rhs.y &&
		lhs.z == rhs.z;
}

template <typename KernelT>
CGAL::Aff_transformation_3<KernelT> build_flatten(
	const CGAL::Direction_3<KernelT> & d) 
{
	typedef CGAL::Vector_3<KernelT> vector_t;

	// use a quaternion

	KernelT::FT ZERO(0.0);
	KernelT::FT ONE(1.0);

	auto dir = normalize(d.vector());

	auto zhat = vector_t(ZERO, ZERO, ONE);
	if (dir == -zhat) {
		dir = vector_t(ZERO, ZERO, ZERO - ONE - ONE); // something arbitrary
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

	return CGAL::Aff_transformation_3<KernelT>(
		ONE-(y*y*s+z*z*s),	x*y*s-w*z*s,		x*z*s+w*y*s,
		x*y*s+w*z*s,		ONE-(x*x*s+z*z*s),	y*z*s-w*x*s,
		x*z*s-w*y*s,		y*z*s+w*x*s,		ONE-(x*x*s+y*y*s));
}