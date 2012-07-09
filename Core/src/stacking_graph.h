#pragma once

#include "stackable.h"

namespace stacking {

namespace impl {

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, stackable, stackable_connection> stacking_graph;
typedef stacking_graph::vertex_descriptor stacking_vertex;

template <typename SpaceFaceRange, typename BlockRange>
stacking_graph create_stacking_graph(SpaceFaceRange * space_faces, const BlockRange & blocks, double connection_height_eps) {
	static_assert(std::is_same<BlockRange::value_type, const block *>::value, "Creating a stacking graph requires a BlockRange value type of const block *.");

	std::vector<stackable> as_stackables;

	// we can't use boost::transform because it requires that its arguments be const
	// we don't want to modify them now, but we're trying to get non-const pointers
	for (auto face = space_faces->begin(); face != space_faces->end(); ++face) {
		as_stackables.push_back(stackable(&*face));
	}
	boost::transform(blocks, std::back_inserter(as_stackables), [](const block * b) { return stackable(b); });

	stacking_graph res(space_faces->size() + blocks.size());

	boost::optional<stackable_connection> connection;
	size_t i = 0;
	for (auto p = as_stackables.begin(); p != as_stackables.end(); ++i, ++p) {
		res[i] = *p;
		size_t j = i;
		for (auto q = p; q != as_stackables.end(); ++j, ++q) {
			if (connection = stackable_connection::do_connect(*p, *q, connection_height_eps)) {
				auto new_edge = boost::add_edge(i, j, res);
				res[new_edge.first] = *connection;
			}
		}
	}

	return res;
}

std::vector<stackable> find_connecting_at_height(stacking_vertex v, const stacking_graph & g, double at, double eps);

} // namespace impl

} // namespace stacking