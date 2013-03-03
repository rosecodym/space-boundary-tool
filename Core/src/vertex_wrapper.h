#pragma once

#include "precompiled.h"

#include "building_graph.h"
#include "layer_information.h"

namespace traversal {

namespace impl {

class vertex_wrapper {
private:
	// Unfortunately, these need to be default-constructible because they're 
	// stored in an std::vector. BUT DON'T DO THAT.
	const building_graph * g_;
	building_graph::vertex_descriptor v_;
	const equality_context * c_;
public:
	typedef boost::optional<double> h_maybe;

	vertex_wrapper(
		building_graph::vertex_descriptor v, 
		const building_graph & g,
		const equality_context & c)
		: g_(&g),
		  v_(v),
		  c_(&c)
	{ }
	
	bool is_halfblock() const { 
		return (*g_)[v_].represents_halfblock(); 
	}
	space_face * as_space_face() const {
		return (*g_)[v_].represents_space_face();
	}
	double thickness() const { return (*g_)[v_].thickness(); }
	const area & vertex_area() const { return (*g_)[v_].a(); }

	std::vector<vertex_wrapper>	adjacent(h_maybe not_at = h_maybe()) const;

	friend class bg_path;
};

} // namespace impl

} // namespace traversal