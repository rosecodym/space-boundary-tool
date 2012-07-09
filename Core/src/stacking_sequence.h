#pragma once

#include "precompiled.h"

#include "blockstack.h"
#include "orientation.h"
#include "stacking_graph.h"
#include "stackable.h"

namespace stacking {

namespace impl {

class stacking_sequence {
private:
	const stacking_graph & g;
	std::vector<stacking_vertex> layers;
	area a;
	NT starting_height;
	const orientation * o;
public:
	stacking_sequence(const stacking_graph & g, stacking_vertex initial, const orientation * o) 
		: g(g), layers(1, initial), a(boost::get<space_face *>(g[initial].data())->face_area()), starting_height(boost::get<space_face *>(g[initial].data())->height()), o(o)
	{ }
	
	const area & sequence_area() const { return a; }
	double total_thickness() const { 
		return std::accumulate(layers.begin(), layers.end(), 0.0, [this](double curr, stacking_vertex layer) {
			return curr + CGAL::to_double(g[layer].thickness());
		});
	}

	stacking_vertex last() const { return layers.back(); }
	size_t layer_count() const { return layers.size(); }

	stacking_sequence split_off(stackable s) {
		stacking_sequence other(*this);
		other.a *= s.stackable_area();
		this->a -= s.stackable_area();
		return other;
	}

	blockstack finish();
};

} // namespace impl

} // namespace stacking