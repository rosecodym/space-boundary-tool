#pragma once

#include "precompiled.h"

class orientation {
private:
	direction_3 m_direction;
	transformation_3 m_flatten;
	transformation_3 m_unflatten;

#ifndef NDEBUG
	double debug_dx;
	double debug_dy;
	double debug_dz;
#endif

	orientation(const orientation & disabled);
	orientation(orientation && disabled);
	orientation & operator = (const orientation & disabled);
	orientation & operator = (orientation && disabled);

public:
	explicit orientation(const direction_3 & d);

	const direction_3 & direction() const { return m_direction; }
	const transformation_3 & flattener() const { return m_flatten; }
	const transformation_3 & unflattener() const { return m_unflatten; }

	const NT & dx() const { return direction().dx(); }
	const NT & dy() const { return direction().dy(); }
	const NT & dz() const { return direction().dz(); }

	vector_3 vector() const { return direction().vector(); }

	std::string to_string() const;

	static bool are_parallel(const orientation & a, const orientation & b) {
		return a.m_direction == b.m_direction || a.m_direction == -b.m_direction;
	}

	static bool are_perpendicular(const orientation & a, const orientation & b, double eps = 0.0);
};

inline bool operator == (const orientation & a, const orientation & b) {
	return a.direction() == b.direction();
}

inline bool operator != (const orientation & a, const orientation & b) {
	return !(a == b);
}