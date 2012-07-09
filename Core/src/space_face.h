#pragma once

#include "precompiled.h"

namespace stacking {

namespace impl {

class space_face {
public:
	bool sense() const;
	NT height() const;
	const area & face_area() const;

	void remove_area(const area & other);
};

} // namespace impl

} // namespace stacking