#include "precompiled.h"

#include "blockstack.h"

#include "stacking_sequence.h"

namespace stacking {

namespace impl {

blockstack stacking_sequence::finish() {
	space_face * initial_space_face = *g[layers.front()].as_space_face();
	if (g[layers.back()].as_space_face()) {
		return blockstack(
			std::move(a), 
			boost::adaptors::slice(layers, 1, layers.size() - 1) |
				boost::adaptors::transformed([this](stacking_vertex v) { return g[v].as_block()->material_layer(); }),
			initial_space_face->sense(),
			o,
			initial_space_face->bounded_space(),
			starting_height,
			(*g[layers.back()].as_space_face())->bounded_space(),
			(*g[layers.back()].as_space_face())->height());
	}
	else {
		return blockstack(
			std::move(a),
			boost::adaptors::slice(layers, 1, layers.size()) |
				boost::adaptors::transformed([this](stacking_vertex v) { return g[v].as_block()->material_layer(); }),
			initial_space_face->sense(),
			o,
			initial_space_face->bounded_space(),
			starting_height);
	}
}

} // namespace impl

} // namespace stacking