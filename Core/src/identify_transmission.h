#pragma once

#include "precompiled.h"

#include "bg_path.h"
#include "block.h"
#include "building_graph.h"
#include "extend_path.h"
#include "report.h"
#include "sbt-core.h"
#include "space.h"
#include "space_face.h"
#include "vertex_wrapper.h"

namespace traversal {

namespace impl {

template <typename SpaceRange>
std::map<const orientation *, std::vector<space_face>> 
get_space_faces_by_orientation(
	const SpaceRange & spaces, 
	equality_context * c) 
{
	std::map<const orientation *, std::vector<space_face>> res;
	for (auto sp = spaces.begin(); sp != spaces.end(); ++sp) {
		auto faces = sp->get_faces(c);
		for (auto oa = faces.begin(); oa != faces.end(); ++oa) {
			res[&oa->orientation()].push_back(space_face(&*sp, *oa));
		}
	}
	return res;
}

template <typename BlockRange>
std::map<const orientation *, std::vector<const block *>> 
get_blocks_by_orientation(const BlockRange & blocks) {
	std::map<const orientation *, std::vector<const block *>> res;
	for (auto b = blocks.begin(); b != blocks.end(); ++b) {
		res[b->block_orientation()].push_back(&*b); 
	}
	return res;
}

template <typename SpaceFaceRange, typename BlockRange>
building_graph create_building_graph(
	SpaceFaceRange * space_faces,
	const BlockRange & blocks,
	const equality_context & c)
{
	static_assert(
		std::is_same<BlockRange::value_type, const block *>::value,
		"The values of the block range passed to create_building_graph must "
		"be pointers to blocks.");

	typedef building_graph::vertex_descriptor vertex;

	building_graph g;
	std::multimap<double, vertex> vertices_by_height;

	reporting::report_progress("Creating building graph");
	
	for (auto f = space_faces->begin(); f != space_faces->end(); ++f) {
		auto v = boost::add_vertex(bg_vertex_data(&*f), g);
		double height = CGAL::to_double(f->height());
		vertices_by_height.insert(std::make_pair(height, v));
	}

	for (auto b = blocks.begin(); b != blocks.end(); ++b) {
		auto v = boost::add_vertex(bg_vertex_data(*b), g);
		auto heights = (*b)->heights();
		double h1 = CGAL::to_double(heights.first);
		vertices_by_height.insert(std::make_pair(h1, v));
		if (heights.second) {
			double h2 = CGAL::to_double(*heights.second);
			vertices_by_height.insert(std::make_pair(h2, v));
		}
	}

	auto first_at_curr_height = vertices_by_height.begin();
	while (first_at_curr_height != vertices_by_height.end()) {
		double curr_height = first_at_curr_height->first;
		auto first_greater_than = vertices_by_height.upper_bound(curr_height);
		double too_far_height = curr_height + c.height_epsilon();
		auto too_far = vertices_by_height.upper_bound(too_far_height);

		for (auto p = first_at_curr_height; p != first_greater_than; ++p) {
			for (auto q = p; q != too_far; ++q) {
				if (p == q) { continue; }
				boost::optional<double> connect_height =
					bg_vertex_data::do_connect(g[p->second], g[q->second], c);
				if (connect_height)
				{
					bg_edge_data data(*connect_height);
					boost::add_edge(p->second, q->second, data, g);
				}
			}
		}

		first_at_curr_height = first_greater_than;
		reporting::report_progress(".");
	}
	
	reporting::report_progress("done.\n");
	return g;
}

template <
	typename SpaceFaceRange, 
	typename BlockRange, 
	typename OutputIterator
>
void process_orientation(
	SpaceFaceRange * space_faces, 
	const BlockRange & blocks, 
	const orientation * o, 
	double max_thickness, 
	const equality_context & c, 
	OutputIterator oi) 
{
	using namespace boost::adaptors;
	typedef double regular_area;
	building_graph g = create_building_graph(space_faces, blocks, c);
	size_t ticks_per_dot = space_faces->size() / 80;
	size_t curr_ticks = 0;
	std::multimap<regular_area, vertex_wrapper> sf_vertices;
	auto all_vertices = boost::vertices(g);
	for (auto v = all_vertices.first; v != all_vertices.second; ++v) {
		vertex_wrapper wrapped(*v, g, c);
		auto sf = wrapped.as_space_face();
		if (sf != nullptr) { 
			double reg_area = CGAL::to_double(sf->starting_regular_area());
			sf_vertices.insert(std::make_pair(reg_area, wrapped));
		}
	}
	reporting::report_progress("Identifying transmission");
	boost::for_each(
		values(sf_vertices) | reversed, 
		[=, &g, &oi, &curr_ticks](vertex_wrapper starting_face) { 
			// Space faces get trimmed during operation so we have to make sure
			// this one didn't get trimmed away entirely previously.
			if (!starting_face.vertex_area().is_empty()) {
				auto t_info_oi = oi; // This rename works around a VS2010 bug
				extend_path(
					bg_path(starting_face),
					starting_face.vertex_area(),
					o,
					max_thickness,
					[&t_info_oi](const transmission_information t_info) {
						*t_info_oi++ = t_info;
					});
				if (++curr_ticks > ticks_per_dot) {
					reporting::report_progress(".");
					curr_ticks = 0;
				}
			}
		});
	reporting::report_progress("done.\n");
}

} // namespace impl

template <typename BlockRange, typename SpaceRange>
std::vector<transmission_information> identify_transmission(
	const BlockRange & blocks,
	const SpaceRange & spaces,
	double max_thickness,
	equality_context * c)
{
	using namespace boost::adaptors;
	using namespace impl;
	typedef boost::format fmt;
	reporting::report_progress(
		fmt(
			"Identifying transmission for %u blocks and %u spaces. Max "
			"thickness is %f.\n") %
		blocks.size() %
		spaces.size() %
		max_thickness);

	equality_context height_c(
		c->height_epsilon() * g_opts.length_units_per_meter);

	auto space_faces = impl::get_space_faces_by_orientation(spaces, c);

	reporting::report_progress(
		fmt("Identified %u relevant orientations.\n") % space_faces.size());

	auto nonfen_blks = 
		get_blocks_by_orientation(
			blocks | filtered([](const block & b) { 
				return !b.is_fenestration(); 
			}));

	typedef std::pair<const orientation * const, std::vector<space_face>> or;
	std::vector<transmission_information> res;
	boost::for_each(space_faces, [&, max_thickness](or & o_info) {
		std::string ostring = o_info.first->to_string().c_str();
		reporting::report_progress(
			fmt("Identifying transmission along %s.\n") % ostring);
		process_orientation(
			&o_info.second, 
			nonfen_blks[o_info.first], 
			o_info.first, 
			max_thickness, 
			height_c, 
			std::back_inserter(res));
		reporting::report_progress("Transmission identified.\n");
	});

	reporting::report_progress(
		fmt("Identified %u transmission sequences.\n") % res.size());
	return res;
}

} // namespace traversal