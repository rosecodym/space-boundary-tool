#pragma once

#include "stackable.h"

namespace stacking {

namespace impl {

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, stackable, stackable_connection> stacking_graph;

template <typename SpaceFaceRange, typename BlockRange>
stacking_graph create_stacking_graph(SpaceFaceRange * space_faces, const BlockRange & blocks, double connection_height_eps, double stack_height_cutoff) {
	std::vector<stackable> as_stackables;
	boost::transform(*space_faces, std::back_inserter(as_stackables), [](space_face & f) { return stackable(&f); });
	boost::transform(blocks, std::back_inserter(as_stackables), [](const block & b) { return stackable(&b); });

	stacking_graph res(space_faces->size() + blocks.size());

	boost::optional<stackable_connection> connection;
	size_t i = 0;
	for (auto p = as_stackables.begin(); p != as_stackables.end(); ++i, ++p) {
		res[i] = *p;
		size_t j = i;
		for (auto q = p; q != as_stackables.end(); ++j, ++q) {
			if (connection = stackable_connection::do_connect(*p, *q, connection_height_eps)) {
				auto res = boost::add_edge(i, j, res);
				res[edge.first] = *connection;
			}
		}
	}

	return res;
}

} // namespace impl

} // namespace stacking