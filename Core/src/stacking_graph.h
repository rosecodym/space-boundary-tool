#pragma once

#include "stackable.h"

#include "printing-macros.h"

class equality_context;

namespace stacking {

namespace impl {

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, stackable, stackable_connection> stacking_graph;
typedef stacking_graph::vertex_descriptor stacking_vertex;

template <typename SpaceFaceRange, typename BlockRange>
stacking_graph create_stacking_graph(SpaceFaceRange * space_faces, const BlockRange & blocks, double connection_height_eps) {
	static_assert(std::is_same<BlockRange::value_type, const block *>::value, "Creating a stacking graph requires a BlockRange value type of const block *.");

	std::vector<stackable> as_stackables;

	NOTIFY_MSG("Creating stacking graph");

	for (auto face = space_faces->begin(); face != space_faces->end(); ++face) {
		stackable s(&*face);
		if (!s.stackable_area().is_empty()) {
			as_stackables.push_back(s);
		}
	}
	for (auto b = blocks.begin(); b != blocks.end(); ++b) {
		stackable s(*b);
		if (!s.stackable_area().is_empty()) {
			as_stackables.push_back(s);
		}
	}

	stacking_graph g(as_stackables.size());

	size_t total_ticks = as_stackables.size() * (as_stackables.size() + 1) / 2;
	size_t ticks_per_dot = total_ticks / 80;
	size_t curr_ticks = 0;

	boost::optional<stackable_connection> connection;
	for (size_t i = 0; i < as_stackables.size(); ++i) {
		g[i] = as_stackables[i];
		for (size_t j = i; j < as_stackables.size(); ++j) {
			if (i != j && (connection = stackable_connection::do_connect(as_stackables[i], as_stackables[j], connection_height_eps))) {
				auto new_edge = boost::add_edge(i, j, g);
				g[new_edge.first] = *connection;
			}
			if (++curr_ticks > ticks_per_dot) {
				NOTIFY_MSG(".");
				curr_ticks = 0;
			}
		}
	}

	NOTIFY_MSG("done.\n");

	return g;
}

std::vector<stacking_vertex> find_connecting_at_height(stacking_vertex v, const stacking_graph & g, double at, double eps);

} // namespace impl

} // namespace stacking