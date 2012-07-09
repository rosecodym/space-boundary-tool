#include "precompiled.h"

#include "blockstack.h"

#include "stacking_sequence.h"

namespace stacking {

namespace impl {

blockstack stacking_sequence::finish() {
	space * initial_space = (*g[layers.front()].as_space_face())->bounded_space();
	if (g[layers.back()].as_space_face()) {
		return blockstack(
			std::move(a), 
			boost::adaptors::slice(layers, 1, layers.size() - 1) |
				boost::adaptors::transformed([this](stacking_vertex v) { return g[v].as_block()->material_layer(); }),
			initial_space,
			(*g[layers.back()].as_space_face())->bounded_space());
	}
	else {
		return blockstack(
			std::move(a),
			boost::adaptors::slice(layers, 1, layers.size()) |
				boost::adaptors::transformed([this](stacking_vertex v) { return g[v].as_block()->material_layer(); }),
			initial_space);
	}
}

} // namespace impl

} // namespace stacking