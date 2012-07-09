#pragma once

#include "blockstack.h"
#include "stackable.h"
#include "stacking_graph.h"
#include "stacking_sequence.h"

namespace stacking {

namespace impl {

struct traversal_visitor : public boost::static_visitor<void> {
	const stacking_graph & graph;
	stacking_sequence * seq;
	double from_height;
	double thickness_cutoff;
	double height_eps;
	std::vector<blockstack> * results;
	traversal_visitor(const stacking_graph & graph, stacking_sequence * seq, double from_height, double thickness_cutoff, double height_eps, std::vector<blockstack> * results)
		: graph(graph), seq(seq), from_height(from_height), thickness_cutoff(thickness_cutoff), height_eps(height_eps), results(results) { }
	void operator () (space_face * f) const;
	void operator () (const block * b) const;
};

void traverse(const stacking_graph & graph, stacking_sequence * curr_sequence, double curr_height, double thickness_cutoff, std::vector<blockstack> * results);

template <typename OutputIterator>
void begin_traversal(const stacking_graph & graph, stacking_graph::vertex_descriptor starting_face, double thickness_cutoff, double height_eps, OutputIterator oi) {
	space_face * as_face = boost::get<space_face *>(graph[starting_face].data());

	stacking_sequence initial_sequence(as_face);
	as_face->clear_area();
	double initial_height = CGAL::to_double(as_face->height());

	std::vector<blockstack> blocks_from_this_face;
	traverse(graph, &initial_sequence, initial_height, thickness_cutoff, height_eps, &blocks_from_this_face);
	boost::copy(blocks_from_this_face, oi);
}

} // namespace impl 

} // namespace stacking