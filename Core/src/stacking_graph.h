#pragma once

#include "stackable.h"

#include "printing-macros.h"

namespace stacking {

namespace impl {

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, stackable, stackable_connection> stacking_graph;
typedef stacking_graph::vertex_descriptor stacking_vertex;

template <typename SpaceFaceRange, typename BlockRange>
stacking_graph create_stacking_graph(SpaceFaceRange * space_faces, const BlockRange & blocks, double connection_height_eps) {
	static_assert(std::is_same<BlockRange::value_type, const block *>::value, "Creating a stacking graph requires a BlockRange value type of const block *.");

	std::vector<stackable> as_stackables;

	PRINT_STACKS("Creating stacking graph.\n");

	// we can't use boost::transform because it requires that its arguments be const
	// we don't want to modify the arguments now, but we're trying to get non-const pointers to them
	for (auto face = space_faces->begin(); face != space_faces->end(); ++face) {
		as_stackables.push_back(stackable(&*face));
	}
	boost::transform(blocks, std::back_inserter(as_stackables), [](const block * b) { return stackable(b); });

	PRINT_STACKS("Got all stackables.\n");

	stacking_graph g(space_faces->size() + blocks.size());

	boost::optional<stackable_connection> connection;
	for (size_t i = 0; i < as_stackables.size(); ++i) {
		g[i] = as_stackables[i];
		for (size_t j = i; j < as_stackables.size(); ++j) {
			if (i != j && (connection = stackable_connection::do_connect(as_stackables[i], as_stackables[j], connection_height_eps))) {
				auto new_edge = boost::add_edge(i, j, g);
				g[new_edge.first] = *connection;
			}
		}
	}

	PRINT_STACKS("Processed all stackables.\n");

	return g;
}

std::vector<stacking_vertex> find_connecting_at_height(stacking_vertex v, const stacking_graph & g, double at, double eps);

} // namespace impl

} // namespace stacking