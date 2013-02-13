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
	using namespace boost::adaptors;
	stacking_graph g = create_stacking_graph(space_faces, blocks, height_eps);
	size_t ticks_per_dot = space_faces->size() / 80;
	size_t curr_ticks = 0;
	std::multimap<NT, stacking_vertex> space_face_vertices;
	auto all_vertices = boost::vertices(g);
	for (auto v = all_vertices.first; v != all_vertices.second; ++v) {
		auto sf = g[*v].as_space_face();
		if (sf) { 
			double reg_area = CGAL::to_double((*sf)->starting_regular_area());
			space_face_vertices.insert(std::make_pair(reg_area, *v));
		}
	}
	reporting::report_progress("Traversing");
	boost::for_each(
		values(space_face_vertices) | reversed, 
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
std::vector<blockstack> build_stacks(
	const BlockRange & blocks, 
	const SpaceRange & spaces, 
	double height_cutoff, 
	equality_context * c) 
{
	using namespace boost::adaptors;
	using namespace impl;
	typedef boost::format fmt;
	reporting::report_progress(
		fmt(
			"Beginning stack construction for %u blocks and %u spaces. Max "
			"stack height is %f.\n") %
		blocks.size() %
		spaces.size() %
		height_cutoff);

	double height_eps = c->height_epsilon() * g_opts.length_units_per_meter;

	auto space_faces = impl::get_space_faces_by_orientation(spaces, c);

	reporting::report_progress(
		fmt("Identified %u relevant orientations.\n") % space_faces.size());

	auto nonfen_blks = 
		get_blocks_by_orientation(
			blocks | filtered([](const block & b) { 
				return !b.is_fenestration(); 
			}));

	typedef std::pair<const orientation * const, std::vector<space_face>> or;
	std::vector<blockstack> res;
	boost::for_each(space_faces, [&, height_cutoff, height_eps](or & o_info) {
		std::string ostring = o_info.first->to_string().c_str();
		reporting::report_progress(
			fmt("Building nonfenestration stacks along %s.\n") % ostring);
		process_group(
			&o_info.second, 
			nonfen_blks[o_info.first], 
			o_info.first, 
			height_cutoff, 
			height_eps, 
			std::back_inserter(res));
		reporting::report_progress("Built stacks.\n");
	});

	reporting::report_progress(fmt("Built %u stacks.\n") % res.size());
	return res;
}

} // namespace stacking