#include "precompiled.h"

#include "stackable.h"

#include "stacking_graph.h"

namespace stacking {

namespace impl {

std::vector<stacking_vertex> find_connecting_at_height(stacking_vertex v, const stacking_graph & g, double at, double eps) {
	std::vector<stacking_vertex> res;
	auto edges = boost::out_edges(v, g);
	for (auto e = edges.first; e != edges.second; ++e) {
		if (equality_context::are_equal(g[*e].connection_height, at, eps)) {
			auto src = boost::source(*e, g);
			res.push_back(src == v ? boost::target(*e, g) : src);
		}
	}
	return res;
}

} // namespace impl

} // namespace stacking