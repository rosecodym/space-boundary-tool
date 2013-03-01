#pragma once

#include "precompiled.h"

#include "area.h"
#include "block.h"
#include "layer_information.h"
#include "space_face.h"

class equality_context;

namespace traversal {

namespace impl {

class bg_vertex_data {
private:
	boost::variant<space_face *, const block *> data_;
public:
	bg_vertex_data() { } // Default ctor for graph property bundle
	explicit bg_vertex_data(space_face * f) : data_(f) { }
	explicit bg_vertex_data(const block * b) : data_(b) { }
	const geometry_2d::area & a() const;
	double thickness() const;
	std::string identifier() const;
	bool represents_halfblock() const;
	space_face * represents_space_face() const;
	boost::optional<layer_information> to_layer() const;

	static boost::optional<double> do_connect(
		bg_vertex_data a, 
		bg_vertex_data b,
		const equality_context & c);
};

// This is a one-property bundle (instead of just storing doubles directly) so 
// that it's clear what the property value represents and so I can extend it 
// later if I need to.
struct bg_edge_data {
	double connection_h;
	bg_edge_data() { } // Default ctor for graph property bundle
	explicit bg_edge_data(double connect_height) 
		: connection_h(connect_height) 
	{ }
};

typedef boost::adjacency_list<
	boost::vecS,
	boost::vecS,
	boost::undirectedS,
	bg_vertex_data,
	bg_edge_data
> building_graph;

} // namespace impl

} // namespace traversal