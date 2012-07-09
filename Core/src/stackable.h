#pragma once

#include "precompiled.h"

#include "block.h"
#include "equality_context.h"
#include "space_face.h"

namespace stacking {

namespace impl {

typedef boost::variant<space_face *, const block *> stackable;

struct stackable_connection {
	double connection_height;
	area connection_area;
	stackable_connection(double height, area && a) : connection_height(height), connection_area(std::move(a)) { }
	static boost::optional<stackable_connection> do_connect(stackable a, stackable b, double height_eps);
};

} // namespace impl

} // namespace stacking