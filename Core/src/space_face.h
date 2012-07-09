#pragma once

#include "precompiled.h"

namespace stacking {

namespace impl {

class space_face {
public:
	bool sense() const;
	NT height() const;
	const area & face_area() const;
};

} // namespace impl

} // namespace stacking