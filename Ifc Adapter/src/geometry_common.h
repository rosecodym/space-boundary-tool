#pragma once

#include "precompiled.h"

#include "number_collection.h"
#include "sbt-ifcadapter.h"

inline vector_3 normalize(const vector_3 & v) {
	return v / sqrt(v.squared_length());
}

inline transformation_3 build_translation(const direction_3 & dir, NT depth) {
	return transformation_3(CGAL::TRANSLATION, normalize(dir.vector()) * depth);
}

inline point_3 from_c_point(point p) {
	return point_3(p.x, p.y, p.z);
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

transformation_3 build_flatten(const direction_3 & d);