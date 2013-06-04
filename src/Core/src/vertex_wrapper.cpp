#include "precompiled.h"

#include "vertex_wrapper.h"

namespace traversal {

namespace impl {

class space_face;

std::vector<vertex_wrapper> vertex_wrapper::adjacent(h_maybe not_at) const 
{
	auto edges = boost::out_edges(v_, *g_);
	std::vector<vertex_wrapper> res;
	for (auto e = edges.first; e != edges.second; ++e) {
		if (!not_at || !c_->is_zero((*g_)[*e].connection_h - *not_at)) {
			auto target = boost::target(*e, *g_);
			res.push_back(vertex_wrapper(target, *g_, *c_));
		}
	}
	return res;
}

} // namespace impl

} // namespace traversal