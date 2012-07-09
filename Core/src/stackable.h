#pragma once

#include "precompiled.h"

#include "block.h"
#include "equality_context.h"
#include "space_face.h"

namespace stacking {

namespace impl {

typedef boost::variant<space_face *, const block *> stackable;

struct stackable_connection {
	double connection_height;
	area connection_area;
	stackable_connection(double height, area && a) : connection_height(height), connection_area(std::move(a)) { }
	static boost::optional<stackable_connection> do_connect(stackable a, stackable b, double height_eps);
};

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, stackable, stackable_connection> stacking_graph_t;

template <typename SpaceFaceRange, typename BlockRange>
stacking_graph_t create_stacking_graph(SpaceFaceRange * space_faces, const BlockRange & blocks, double connection_height_eps, double stack_height_cutoff) {
	std::vector<stackable> as_stackables;
	boost::transform(*space_faces, std::back_inserter(as_stackables), [](space_face & f) { return stackable(&f); });
	boost::transform(blocks, std::back_inserter(as_stackables), [](const block & b) { return stackable(&b); });

	stacking_graph_t res(space_faces->size() + blocks.size());

	boost::optional<stackable_connection> connection;
	size_t i = 0;
	for (auto p = as_stackables.begin(); p != as_stackables.end(); ++i, ++p) {
		size_t j = i;
		for (auto q = p; q != as_stackables.end(); ++j, ++q) {
			if (connection = stackable_connection::do_connect(*p, *q, connection_height_eps)) {
				auto res = boost::add_edge(i, j, res);
				res[edge.first].connection_height = connection->connection_height;
				res[edge.first].connection_area = connection->connection_area;
			}
		}
	}

	return res;
}

} // namespace impl

} // namespace stacking