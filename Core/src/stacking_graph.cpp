#include "precompiled.h"

#include "printing-macros.h"
#include "stackable.h"

#include "stacking_graph.h"

namespace stacking {

namespace impl {

std::vector<stacking_vertex> find_connecting_at_height(stacking_vertex v, const stacking_graph & g, double at, double eps) {
	PRINT_STACKS("Entered find connecting_at_height (height = %f).\n", at);
	std::vector<stacking_vertex> res;
	auto edges = boost::out_edges(v, g);
	PRINT_STACKS("Got %u connected edges.\n", std::distance(edges.first, edges.second));
	boost::copy(
		boost::make_iterator_range(edges.first, edges.second) | 
		boost::adaptors::filtered([&g, at, eps](stacking_graph::edge_descriptor connected) {
			return equality_context::are_equal(g[connected].connection_height, at, eps);
		}) |
		boost::adaptors::transformed([v, &g](stacking_graph::edge_descriptor e) -> stacking_vertex {
			auto src = boost::source(e, g);
			return src == v ? boost::target(e, g) : src;
		}),
		std::back_inserter(res));
	PRINT_STACKS("Exiting find_connecting_at_height.\n");
	return res;
}

} // namespace impl

} // namespace stacking