#include "precompiled.h"

#include "stackable.h"

#include "stacking_graph.h"

namespace stacking {

namespace impl {

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