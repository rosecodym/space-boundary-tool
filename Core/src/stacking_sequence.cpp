#include "precompiled.h"

#include "stacking_sequence.h"

#include "blockstack.h"
#include "layer_information.h"
#include "sbt-core.h"

namespace stacking {

namespace impl {

blockstack stacking_sequence::finish(bool too_thick) {
	auto get_mat_layers = 
		[this](size_t first, size_t beyond) -> std::vector<layer_information> {
			std::vector<layer_information> res;
			for (size_t i = first; i < beyond; ++i) {
				res.push_back(g[layers[i]].as_block()->material_layer());
			}
			return res;
		};

	space_face * initial_space_face = *g[layers.front()].as_space_face();
	if (g[layers.back()].as_space_face()) {
		return blockstack(
			std::move(a), 
			get_mat_layers(1, layers.size() - 1),
			initial_space_face->sense(),
			o,
			false,
			initial_space_face->bounded_space(),
			starting_height,
			(*g[layers.back()].as_space_face())->bounded_space(),
			(*g[layers.back()].as_space_face())->height());
	}
	else {
		return blockstack(
			std::move(a),
			get_mat_layers(1, layers.size()),
			initial_space_face->sense(),
			o,
			!too_thick && g[layers.back()].as_block()->material_layer().has_both_sides(),
			initial_space_face->bounded_space(),
			starting_height);
	}
}

} // namespace impl

} // namespace stacking