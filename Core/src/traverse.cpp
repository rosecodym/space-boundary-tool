#include "precompiled.h"

#include "blockstack.h"
#include "equality_context.h"
#include "stacking_graph.h"
#include "stacking_sequence.h"

#include "traverse.h"

namespace stacking {

namespace impl {

void traversal_visitor::operator () (space_face * f) const {
	stacking_sequence other = seq->split_off(stackable(f));
	f->remove_area(other.sequence_area());
	results->push_back(other.finish()); // terminated because of end space face
}

void traversal_visitor::operator () (const block * b) const {
	stacking_sequence other = seq->split_off(stackable(b));
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

void traverse(const stacking_graph & graph, stacking_sequence * curr_sequence, double curr_height, double thickness_cutoff, double height_eps, std::vector<blockstack> * results) {
	if (curr_sequence->total_thickness() > thickness_cutoff) {
		results->push_back(curr_sequence->finish()); // terminated because of too thick
		return;
	}
	auto connecting = find_connecting_at_height(curr_sequence->last(), graph, curr_height, height_eps);
	boost::for_each(connecting, [&graph, curr_sequence, curr_height, thickness_cutoff, height_eps, results](stackable connected) {
		boost::apply_visitor(traversal_visitor(graph, curr_sequence, curr_height, thickness_cutoff, height_eps, results), connected.data());
	});
	if (!curr_sequence->sequence_area().is_empty()) {
		results->push_back(curr_sequence->finish()); // terminated because of no other terminus (probably an incomplete building definition)
	}
}

} // namespace impl

} // namespace stacking