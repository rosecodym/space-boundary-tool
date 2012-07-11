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
	results->push_back(other.finish()); // terminated because of end space face
}

void traversal_visitor::operator () (const block * b) const {
	stacking_sequence other = seq->split_off(next_vertex);
	if (!b->heights().second) {
		results->push_back(other.finish()); // terminated because hit a 5th-level block
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

void traverse(const stacking_graph & g, stacking_sequence * curr_sequence, double curr_height, double thickness_cutoff, double height_eps, std::vector<blockstack> * results) {
	if (curr_sequence->total_thickness() > thickness_cutoff) {
		results->push_back(curr_sequence->finish()); // terminated because of too thick
		return;
	}
	auto connecting = find_connecting_at_height(curr_sequence->last(), g, curr_height, height_eps);
	PRINT_STACKS("Found %u connecting stackables.\n", connecting.size());
	boost::for_each(connecting, [&g, curr_sequence, curr_height, thickness_cutoff, height_eps, results](stacking_vertex connected) {
		boost::apply_visitor(traversal_visitor(g, connected, curr_sequence, curr_height, thickness_cutoff, height_eps, results), g[connected].data());
	});
	if (curr_sequence->layer_count() > 1 && !curr_sequence->sequence_area().is_empty()) { // layer_count can be 1 if the building geometry is incomplete
		results->push_back(curr_sequence->finish()); // terminated because of external
	}
}

} // namespace impl

} // namespace stacking