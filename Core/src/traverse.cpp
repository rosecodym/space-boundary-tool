#include "precompiled.h"

#include "blockstack.h"
#include "equality_context.h"
#include "stacking_graph.h"
#include "stacking_sequence.h"

#include "traverse.h"

namespace stacking {

namespace impl {

void traversal_visitor::operator () (space_face * f) const {
	stacking_sequence other = seq->split_off(next_vertex);
	f->remove_area(other.sequence_area());
	if (!other.sequence_area().is_empty()) {
		results->push_back(other.finish(false));
		PRINT_STACKS("Terminating stack - found opposite space face.\n");
	}
	else {
		PRINT_STACKS("Dropping stack - empty area after final space face.\n");
	}
}

void traversal_visitor::operator () (const block * b) const {
	stacking_sequence other = seq->split_off(next_vertex);
	if (!other.sequence_area().is_empty()) {
		if (!b->heights().second) {
			results->push_back(other.finish(false));
			PRINT_STACKS("Terminating stack - at halfblock.\n");
		}
		else {
			if (equality_context::are_equal(from_height, b->heights().first, height_eps)) {
				traverse(graph, &other, CGAL::to_double(*b->heights().second), thickness_cutoff, height_eps, results);
			}
			else {
				traverse(graph, &other, CGAL::to_double(b->heights().first), thickness_cutoff, height_eps, results);
			}
		}
	}
	else {
		PRINT_STACKS("Dropping stack - empty.\n");
	}
}

void traverse(const stacking_graph & g, stacking_sequence * curr_sequence, double curr_height, double thickness_cutoff, double height_eps, std::vector<blockstack> * results) {
	PRINT_STACKS("Entering traverse.\n");
	if (curr_sequence->total_thickness() > thickness_cutoff) {
		results->push_back(curr_sequence->finish(true));
		PRINT_STACKS("Terminating stack - too thick.\n");
		return;
	}
	PRINT_STACKS("Finding connecting stackables.\n");
	auto connecting = find_connecting_at_height(curr_sequence->last(), g, curr_height, height_eps);
	PRINT_STACKS("Found %u connecting stackables.\n", connecting.size());
	boost::for_each(connecting, [&g, curr_sequence, curr_height, thickness_cutoff, height_eps, results](stacking_vertex connected) {
		boost::apply_visitor(traversal_visitor(g, connected, curr_sequence, curr_height, thickness_cutoff, height_eps, results), g[connected].data());
	});
	if (curr_sequence->layer_count() > 1 && !curr_sequence->sequence_area().is_empty()) { // layer_count can be 1 if the building geometry is incomplete
		results->push_back(curr_sequence->finish(false));
		PRINT_STACKS("Terminating stack - at building facade.\n");
	}
	else {
		PRINT_STACKS("Dropping stack - empty or single layer.\n");
	}
}

} // namespace impl

} // namespace stacking