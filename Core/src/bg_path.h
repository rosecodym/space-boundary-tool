#pragma once

#include "precompiled.h"

#include "building_graph.h"
#include "layer_information.h"
#include "vertex_wrapper.h"

namespace traversal {

namespace impl {

class bg_path {
private:
	const building_graph & g_;
	std::vector<building_graph::vertex_descriptor> vertices_;
	std::vector<building_graph::edge_descriptor> edges_;
	const equality_context & c_;
public:
	explicit bg_path(vertex_wrapper start)
		: g_(*start.g_),
		  vertices_(1, start.v_),
		  c_(*start.c_)
	{ }

	int length() const { return edges_.size(); }

	space_face * start_face() const {
		return g_[vertices_.front()].represents_space_face();
	}
	vertex_wrapper end_vertex() const { 
		return vertex_wrapper(vertices_.back(), g_, c_);
	}
	double last_edge_h() const { return g_[edges_.back()].connection_h; }
	double total_thickness() const {
		double tot = 0.0;
		for (auto v = vertices_.begin(); v != vertices_.end(); ++v) {
			tot += g_[*v].thickness();
		}
		return tot;
	}

	bg_path with_appended(vertex_wrapper v) const {
		bg_path copy(*this);
		building_graph::edge_descriptor e;
		bool exists;
		std::tie(e, exists) = boost::lookup_edge(vertices_.back(), v.v_, g_);
		assert(exists);
		copy.vertices_.push_back(v.v_);
		copy.edges_.push_back(e);
		return copy;
	}

	std::vector<layer_information> to_layers() const {
		std::vector<layer_information> res;
		for (auto v = vertices_.begin(); v != vertices_.end(); ++v) {
			auto as_layer = g_[*v].to_layer();
			if (as_layer) { res.push_back(*as_layer); }
		}
		return res;
	}
};

} // namespace impl

} // namespace traversal