#pragma once

#include "precompiled.h"

class space;

namespace stacking {

namespace impl {

class space_face {
private:
	bool m_sense;
	NT m_height;
	area m_area;
	area m_original_area;
	const space * m_space;
public:
	space_face(const space * sp, const oriented_area & geometry) 
		: m_sense(geometry.sense()), m_height(geometry.height()), m_area(geometry.area_2d()), m_original_area(geometry.area_2d()), m_space(sp) { }

	bool sense() const { return m_sense; }
	NT height() const { return m_height; }
	const area & face_area() const { return m_area; }
	const space * bounded_space() const { return m_space; }

	void remove_area(const area & other) { m_area -= other; }
	void reset_area_to_original() { m_area = m_original_area; }
};

} // namespace impl

} // namespace stacking