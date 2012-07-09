#pragma once

#include "precompiled.h"

#include "blockstack.h"
#include "equality_context.h"
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
		boost::for_each(sp->get_faces(c), [&res, sp](const oriented_area & geom) {
			res[&geom.orientation()].push_back(space_face(&*sp, geom));
		});
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
void process_group(SpaceFaceRange * space_faces, const BlockRange & blocks, double height_cutoff, double height_eps, OutputIterator oi) {
	stacking_graph g = create_stacking_graph(space_faces, blocks, height_eps);
	auto all_vertices = boost::vertices(g);
	boost::for_each(
		boost::make_iterator_range(all_vertices.first, all_vertices.second) | boost::adaptors::filtered([&g](stacking_vertex v) { return g[v].as_space_face(); }),
		[&g, &blocks, height_cutoff, height_eps, &oi](stacking_vertex starting_face) { 
			begin_traversal(g, starting_face, (*blocks.begin())->block_orientation(), height_cutoff, height_eps, oi); 
		});
}

} // namespace impl

template <typename BlockRange, typename SpaceRange>
std::vector<blockstack> build_stacks(const BlockRange & blocks, const SpaceRange & spaces, double height_cutoff, equality_context * c) {
	auto space_faces = impl::get_space_faces_by_orientation(spaces, c);

	auto fenestration_blocks = impl::get_blocks_by_orientation(blocks | boost::adaptors::filtered([](const block & b) { return b.is_fenestration(); }));
	auto nonfenestration_blocks = impl::get_blocks_by_orientation(blocks | boost::adaptors::filtered([](const block & b) { return !b.is_fenestration(); }));

	std::vector<blockstack> res;
	boost::for_each(space_faces, [&res, &space_faces, &fenestration_blocks, height_cutoff, c](std::pair<const orientation * const, std::vector<impl::space_face>> & o_info) {
		impl::process_group(&o_info.second, fenestration_blocks[o_info.first], height_cutoff, c->height_epsilon(), std::back_inserter(res));
	});
	boost::for_each(space_faces, [](std::pair<const orientation * const, std::vector<impl::space_face>> & o_info) {
		boost::for_each(o_info.second, [](impl::space_face & f) { f.reset_area_to_original(); });
	});
	boost::for_each(space_faces, [&res, &space_faces, &nonfenestration_blocks, height_cutoff, c](std::pair<const orientation * const, std::vector<impl::space_face>> & o_info) {
		impl::process_group(&o_info.second, nonfenestration_blocks[o_info.first], height_cutoff, c->height_epsilon(), std::back_inserter(res));
	});

	return res;
}

} // namespace stacking