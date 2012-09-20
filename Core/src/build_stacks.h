#pragma once

#include "precompiled.h"

#include "blockstack.h"
#include "equality_context.h"
#include "report.h"
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
void process_group(SpaceFaceRange * space_faces, const BlockRange & blocks, const orientation * o, double height_cutoff, double height_eps, OutputIterator oi) {
	stacking_graph g = create_stacking_graph(space_faces, blocks, height_eps);
	auto all_vertices = boost::vertices(g);
	size_t ticks_per_dot = space_faces->size() / 80;
	size_t curr_ticks = 0;
	reporting::report_progress("Traversing");
	boost::for_each(
		boost::make_iterator_range(all_vertices.first, all_vertices.second) | boost::adaptors::filtered([&g](stacking_vertex v) { return g[v].as_space_face(); }),
		[&g, o, height_cutoff, height_eps, &oi, &curr_ticks, ticks_per_dot](stacking_vertex starting_face) { 
			begin_traversal(g, starting_face, o, height_cutoff, height_eps, oi);
			if (++curr_ticks > ticks_per_dot) {
				reporting::report_progress(".");
				curr_ticks = 0;
			}
		});
	reporting::report_progress("done.\n");
}

} // namespace impl

template <typename BlockRange, typename SpaceRange>
std::vector<blockstack> build_stacks(const BlockRange & blocks, const SpaceRange & spaces, double height_cutoff, equality_context * c) {
	reporting::report_progress(boost::format("Beginning stack construction for %u blocks and %u spaces. Max stack height is %f.\n") %
		blocks.size() %
		spaces.size() %
		height_cutoff);

	double height_eps = c->height_epsilon();

	auto space_faces = impl::get_space_faces_by_orientation(spaces, c);

	reporting::report_progress(boost::format("Identified %u relevant orientations.\n") % space_faces.size());

	auto fenestration_blocks = impl::get_blocks_by_orientation(blocks | boost::adaptors::filtered([](const block & b) { return b.is_fenestration(); }));
	auto nonfenestration_blocks = impl::get_blocks_by_orientation(blocks | boost::adaptors::filtered([](const block & b) { return !b.is_fenestration(); }));

	std::vector<blockstack> res;
	boost::for_each(space_faces, [&res, &space_faces, &fenestration_blocks, height_cutoff, height_eps](std::pair<const orientation * const, std::vector<impl::space_face>> & o_info) {
		reporting::report_progress(boost::format("Building fenestration stacks along %s.\n") % o_info.first->to_string().c_str());
		impl::process_group(&o_info.second, fenestration_blocks[o_info.first], o_info.first, height_cutoff, height_eps, std::back_inserter(res));
		reporting::report_progress("Built stacks.\n");
	});
	reporting::report_progress("Resetting space faces");
	boost::for_each(space_faces, [](std::pair<const orientation * const, std::vector<impl::space_face>> & o_info) {
		boost::for_each(o_info.second, [](impl::space_face & f) { f.reset_area_to_original(); reporting::report_progress("."); });
	});
	reporting::report_progress("done.\n");
	boost::for_each(space_faces, [&res, &space_faces, &nonfenestration_blocks, height_cutoff, height_eps](std::pair<const orientation * const, std::vector<impl::space_face>> & o_info) {
		reporting::report_progress(boost::format("Building nonfenestration stacks along %s.\n") % o_info.first->to_string().c_str());
		impl::process_group(&o_info.second, nonfenestration_blocks[o_info.first], o_info.first, height_cutoff, height_eps, std::back_inserter(res));
		reporting::report_progress("Built stacks.\n");
	});

	reporting::report_progress(boost::format("Built %u stacks.\n") % res.size());
	return res;
}

} // namespace stacking