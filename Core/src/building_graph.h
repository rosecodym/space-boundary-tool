#pragma once

#include "precompiled.h"

#include "area.h"
#include "block.h"
#include "layer_information.h"
#include "report.h"
#include "space_face.h"

class equality_context;

namespace traversal {

namespace impl {

class bg_vertex_data {
private:
	boost::variant<space_face *, const block *> data_;
	// I store area here during construction instead of retrieving it from
	// data_ on-demand to make it easier in the future to flatten it if that
	// becomes necessary.
	geometry_2d::area area_;
public:
	bg_vertex_data() { } // Default ctor for graph property bundle
	explicit bg_vertex_data(space_face * f) 
		: data_(f),
		  area_(f->face_area())
	{ }
	explicit bg_vertex_data(const block * b) 
		: data_(b),
		  area_(b->base_area())
	{ }
	const geometry_2d::area & a() const { return area_; }
	double thickness() const;
	std::string identifier() const;
	bool represents_halfblock() const;
	space_face * represents_space_face() const;
	boost::optional<layer_information> to_layer() const;

	static boost::optional<double> do_connect(
		bg_vertex_data a, 
		bg_vertex_data b,
		const equality_context & c);
};

// This is a one-property bundle (instead of just storing doubles directly) so 
// that it's clear what the property value represents and so I can extend it 
// later if I need to.
struct bg_edge_data {
	double connection_h;
	bg_edge_data() { } // Default ctor for graph property bundle
	explicit bg_edge_data(double connect_height) 
		: connection_h(connect_height) 
	{ }
};

typedef boost::adjacency_list<
	boost::vecS,
	boost::vecS,
	boost::undirectedS,
	bg_vertex_data,
	bg_edge_data
> building_graph;

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

} // namespace impl

} // namespace traversal