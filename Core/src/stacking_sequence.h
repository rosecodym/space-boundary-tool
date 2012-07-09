#pragma once

#include "precompiled.h"

#include "blockstack.h"
#include "stackable.h"

namespace stacking {

namespace impl {

class stacking_sequence {
private:
	std::vector<stackable> layers;
	area a;
public:
	stacking_sequence(space_face * initial) : layers(1, stackable(initial)), a(initial->face_area()) { }
	
	const area & sequence_area() const { return a; }
	double total_thickness() const;

	stacking_sequence split_off(stackable s);
	blockstack finish() const;
};

} // namespace impl

} // namespace stacking