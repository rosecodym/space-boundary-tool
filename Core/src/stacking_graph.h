#pragma once

#include "stackable.h"

#include "printing-macros.h"

class equality_context;

namespace stacking {

namespace impl {

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, stackable, stackable_connection> stacking_graph;
typedef stacking_graph::vertex_descriptor stacking_vertex;

template <typename SpaceFaceRange, typename BlockRange>
stacking_graph create_stacking_graph(SpaceFaceRange * space_faces, const BlockRange & blocks, double connection_height_eps) {
	static_assert(std::is_same<BlockRange::value_type, const block *>::value, "Creating a stacking graph requires a BlockRange value type of const block *.");

	std::vector<stackable> as_stackables;
	std::multimap<double, size_t> indices_by_height;

	NOTIFY_MSG("Creating stacking graph");

	for (auto face = space_faces->begin(); face != space_faces->end(); ++face) {
		stackable s(&*face);
		// i'm not sure why there's an empty check
		if (!s.stackable_area().is_empty()) {
			indices_by_height.insert(std::make_pair(CGAL::to_double(face->height()), as_stackables.size()));
			as_stackables.push_back(s);
		}
	}
	for (auto b = blocks.begin(); b != blocks.end(); ++b) {
		stackable s(*b);
		// i'm not sure why there's an empty check
		if (!s.stackable_area().is_empty()) {
			auto heights = (*b)->heights();
			indices_by_height.insert(std::make_pair(CGAL::to_double(heights.first), as_stackables.size()));
			if (heights.second) {
				indices_by_height.insert(std::make_pair(CGAL::to_double(*heights.second), as_stackables.size()));
			}
			as_stackables.push_back(s);
		}
	}

	stacking_graph g(as_stackables.size());
	for (size_t i = 0; i < as_stackables.size(); ++i) {
		g[i] = as_stackables[i];
	}

	boost::optional<stackable_connection> connection;

	auto first_at_curr_height = indices_by_height.begin();
	while (first_at_curr_height != indices_by_height.end()) {
		double curr_height = first_at_curr_height->first;
		auto first_greater_than = indices_by_height.upper_bound(curr_height);
		auto too_far = indices_by_height.upper_bound(curr_height + connection_height_eps);

		for (auto p = first_at_curr_height; p != first_greater_than; ++p) {
			for (auto q = p; q != too_far; ++q) {
				if (p != q && (connection = stackable_connection::do_connect(as_stackables[p->second], as_stackables[q->second], connection_height_eps))) {
					auto new_edge = boost::add_edge(p->second, q->second, g);
					g[new_edge.first] = *connection;
				}
			}
		}

		first_at_curr_height = first_greater_than;
		NOTIFY_MSG(".");
	}

	NOTIFY_MSG("done.\n");

	return g;
}

std::vector<stacking_vertex> find_connecting_at_height(stacking_vertex v, const stacking_graph & g, double at, double eps);

} // namespace impl

} // namespace stacking