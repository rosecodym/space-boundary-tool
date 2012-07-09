#pragma once

#include "stackable.h"

namespace stacking {

namespace impl {

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, stackable, stackable_connection> stacking_graph;
typedef stacking_graph::vertex_descriptor stacking_vertex;

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

std::vector<stackable> find_connecting_at_height(stacking_vertex v, const stacking_graph & g, double at, double eps) {
	std::vector<stackable> res;
	auto edges = boost::out_edges(v, g);
	boost::copy(
		boost::make_iterator_range(edges.first, edges.second) | 
		boost::adaptors::filtered([&g, at, eps](stacking_graph::edge_descriptor connected) {
			return equality_context::are_equal(g[connected].connection_height, at, eps);
		}) |
		boost::adaptors::transformed([v, g](stacking_graph::edge_descriptor e) -> stackable {
			auto src = boost::source(e, g);
			return src == v ? g[boost::target(e, g)] : g[src];
		}),
		std::back_inserter(res));
	return res;
}

} // namespace impl

} // namespace stacking