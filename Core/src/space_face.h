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
	space * m_space;
public:
	bool sense() const { return m_sense; }
	NT height() const { return m_height; }
	const area & face_area() const { return m_area; }
	space * bounded_space() const { return m_space; }

	void remove_area(const area & other) { m_area -= other; }
	void reset_area_to_original() { m_area = m_original_area; }
};

} // namespace impl

} // namespace stacking