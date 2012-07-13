#pragma once

#include "precompiled.h"

#include "blockstack.h"
#include "equality_context.h"
#include "printing-macros.h"
#include "space.h"
#include "space_face.h"
#include "stacking_graph.h"
#include "traverse.h"

class orientation;

namespace stacking {

namespace impl {

template <typename SpaceRange>
std::map<const orientation *, std::vector<space_face>> get_space_faces_by_orientation(const SpaceRange & spaces, equality_context * c) {
	std::map<const orientation *, std::vector<space_face>> res;
	for (auto sp = spaces.begin(); sp != spaces.end(); ++sp) {
		PRINT_STACKS("Organizing space faces for %s.\n", sp->global_id().c_str());
		boost::for_each(sp->get_faces(c), [&res, sp](const oriented_area & geom) {
			res[&geom.orientation()].push_back(space_face(&*sp, geom));
		});
		PRINT_STACKS("Organized space faces for %s.\n", sp->global_id().c_str());
	}
	return res;
}

template <typename BlockRange>
std::map<const orientation *, std::vector<const block *>> get_blocks_by_orientation(const BlockRange & blocks) {
	std::map<const orientation *, std::vector<const block *>> res;
	boost::for_each(blocks, [&res](const block & b) { res[b.block_orientation()].push_back(&b); });
	return res;
}

template <typename SpaceFaceRange, typename BlockRange, typename OutputIterator>
void process_group(SpaceFaceRange * space_faces, const BlockRange & blocks, const orientation * o, double height_cutoff, double height_eps, equality_context * local_c, OutputIterator oi) {
	stacking_graph g = create_stacking_graph(space_faces, blocks, height_eps, local_c);
	auto all_vertices = boost::vertices(g);
	size_t ticks_per_dot = space_faces->size() / 80;
	size_t curr_ticks = 0;
	NOTIFY_MSG("Traversing");
	boost::for_each(
		boost::make_iterator_range(all_vertices.first, all_vertices.second) | boost::adaptors::filtered([&g](stacking_vertex v) { return g[v].as_space_face(); }),
		[&g, o, height_cutoff, height_eps, &oi, &curr_ticks, ticks_per_dot](stacking_vertex starting_face) { 
			begin_traversal(g, starting_face, o, height_cutoff, height_eps, oi);
			if (++curr_ticks > ticks_per_dot) {
				NOTIFY_MSG(".");
				curr_ticks = 0;
			}
		});
	NOTIFY_MSG("done.\n");
}

} // namespace impl

template <typename BlockRange, typename SpaceRange>
std::vector<blockstack> build_stacks(const BlockRange & blocks, const SpaceRange & spaces, double height_cutoff, equality_context * c) {
	NOTIFY_MSG("Beginning stack construction for %u blocks and %u spaces. Max stack height is %f.\n", blocks.size(), spaces.size(), height_cutoff);

	double height_eps = c->height_epsilon();

	auto space_faces = impl::get_space_faces_by_orientation(spaces, c);

	NOTIFY_MSG("Identified %u relevant orientations.\n", space_faces.size());

	auto fenestration_blocks = impl::get_blocks_by_orientation(blocks | boost::adaptors::filtered([](const block & b) { return b.is_fenestration(); }));
	auto nonfenestration_blocks = impl::get_blocks_by_orientation(blocks | boost::adaptors::filtered([](const block & b) { return !b.is_fenestration(); }));

	std::map<const orientation *, std::unique_ptr<equality_context>> orientation_area_contexts;
	boost::transform(space_faces, std::inserter(orientation_area_contexts, orientation_area_contexts.begin()), [c](const std::pair<const orientation *, std::vector<impl::space_face>> & info) {
		return std::make_pair(info.first, std::unique_ptr<equality_context>(new equality_context(c->area_epsilon())));
	});

	std::vector<blockstack> res;
	boost::for_each(space_faces, [&res, &space_faces, &fenestration_blocks, height_cutoff, &orientation_area_contexts, height_eps](std::pair<const orientation * const, std::vector<impl::space_face>> & o_info) {
		NOTIFY_MSG("Building fenestration stacks along %s.\n", o_info.first->to_string().c_str());
		impl::process_group(&o_info.second, fenestration_blocks[o_info.first], o_info.first, height_cutoff, height_eps, orientation_area_contexts[o_info.first].get(), std::back_inserter(res));
		NOTIFY_MSG("Built stacks.\n");
	});
	NOTIFY_MSG("Resetting space faces");
	boost::for_each(space_faces, [](std::pair<const orientation * const, std::vector<impl::space_face>> & o_info) {
		boost::for_each(o_info.second, [](impl::space_face & f) { f.reset_area_to_original(); NOTIFY_MSG("."); });
	});
	NOTIFY_MSG("done.\n");
	boost::for_each(space_faces, [&res, &space_faces, &nonfenestration_blocks, height_cutoff, &orientation_area_contexts, height_eps](std::pair<const orientation * const, std::vector<impl::space_face>> & o_info) {
		NOTIFY_MSG("Building nonfenestration stacks along %s.\n", o_info.first->to_string().c_str());
		impl::process_group(&o_info.second, nonfenestration_blocks[o_info.first], o_info.first, height_cutoff, height_eps, orientation_area_contexts[o_info.first].get(), std::back_inserter(res));
		NOTIFY_MSG("Built stacks.\n");
	});

	NOTIFY_MSG("Built %u stacks.\n", res.size());
	return res;
}

} // namespace stacking