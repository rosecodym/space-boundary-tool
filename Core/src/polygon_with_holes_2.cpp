#include "precompiled.h"

#include "cgal-util.h"
#include "geometry_common.h"
#include "printing-macros.h"
#include "sbt-core.h"

#include "polygon_with_holes_2.h"

extern sb_calculation_options g_opts;

namespace {

typedef std::vector<polygon_2>::const_iterator hole_iter;
typedef polygon_2::Vertex_circulator point_circ;

struct annotated_edge {
	segment_2 seg;
	point_circ a;
	point_circ b;
	annotated_edge() { }
	annotated_edge(point_circ a, point_circ b) : seg(*a, *b), a(a), b(b) { }
};

typedef std::vector<annotated_edge> edgelist_t;

bool intersects_another_edge(const edgelist_t edges, const annotated_edge & e) {
	return boost::find_if(edges, [&e](const annotated_edge & other) { 
		return (e.a != other.a && e.a != other.b && e.b != other.a && e.b != other.b && CGAL::do_intersect(e.seg, other.seg));
	}) != edges.end();
}

edgelist_t build_edges(const polygon_with_holes_2 & pwh) {
	edgelist_t results;
	auto q = pwh.outer().vertices_circulator();
	auto p = q++;
	do {
		results.push_back(annotated_edge(p++, q++));
	}
	while (p != pwh.outer().vertices_circulator());
	boost::for_each(pwh.holes(), [&p, &q, &results](const polygon_2 & hole) {
		q = hole.vertices_circulator();
		p = q++;
		do {
			results.push_back(annotated_edge(p++, q++));
		}
		while (p != hole.vertices_circulator());
	});
	return results;
}

bool identify_split_points(const edgelist_t & all_edges, const polygon_2 & poly_a, const polygon_2 & poly_b, point_circ * a, point_circ * b, point_circ * c, point_circ * d) {
	*a = poly_a.vertices_circulator();
	annotated_edge candidate;
	do {
		*b = poly_b.vertices_circulator();
		do {
			candidate = annotated_edge(*a, *b);
			if (!intersects_another_edge(all_edges, candidate)) {
				*c = *b;
				++*c;
				do {
					*d = *a;
					--*d;
					do {
						annotated_edge candidate2(*c, *d);
						if (!intersects_another_edge(all_edges, candidate2) && !CGAL::do_intersect(candidate.seg, candidate2.seg)) {
							return true;
						}
						--*d;
					}
					while (*d != *a);
					++*c;
				}
				while (*c != *b);
			}
			++*b;
		}
		while (*b != poly_b.vertices_circulator());
		++*a;
	}
	while (*a != poly_a.vertices_circulator());
	return false;
}

bool identify_split_points(const polygon_with_holes_2 & pwh, point_circ * a, point_circ * b, point_circ * c, point_circ * d, hole_iter * h_ad, hole_iter * h_bc) {
	auto all_edges = build_edges(pwh);
	for (auto p = pwh.holes().begin(); p != pwh.holes().end(); ++p) {
		for (auto q = pwh.holes().begin(); q != pwh.holes().end(); ++q) {
			if (p != q && identify_split_points(all_edges, *p, *q, a, b, c, d)) {
				*h_ad = p;
				*h_bc = q;
				return true;
			}
		}
	}
	for (auto p = pwh.holes().begin(); p != pwh.holes().end(); ++p) {
		if (identify_split_points(all_edges, pwh.outer(), *p, a, b, c, d)) {
			*h_ad = pwh.holes().end();
			*h_bc = p;
			return true;
		}
	}
	return false;
}

polygon_2 extract_section(polygon_with_holes_2 * pwh) {
	if (pwh->holes().size() == 0) {
		return pwh->outer();
	}

	point_circ a, b, c, d;
	hole_iter h_ad, h_bc;
	if (!identify_split_points(*pwh, &a, &b, &c, &d, &h_ad, &h_bc)) {
		// uh oh, something didn't work
		*pwh = polygon_with_holes_2(pwh->outer(), std::vector<polygon_2>()); // remove all the holes so the algorithm terminates
		return pwh->outer();
	}

	polygon_2 res;
	res.push_back(*a);
	point_circ curr = b;
	while (curr != c) {
		res.push_back(*curr++);
	}
	res.push_back(*c);
	curr = d;
	while (curr != a) {
		res.push_back(*curr++);
	}

	polygon_2 new_poly;
	curr = d;
	while (curr != a) {
		new_poly.push_back(*curr--);
	}
	new_poly.push_back(*a);
	curr = b;
	while (curr != c) {
		new_poly.push_back(*curr--);
	}
	new_poly.push_back(*c);
	
	const polygon_2 * new_outer;
	std::vector<const polygon_2 *> new_holes;

	if (h_ad != pwh->holes().end()) {
		// it was two holes
		for (auto p = pwh->holes().begin(); p != pwh->holes().end(); ++p) {
			if (p != h_bc) {
				new_holes.push_back(&*p);
			}
			new_holes.push_back(&new_poly);
		}
		new_outer = &pwh->outer();
	}
	else {
		// it was the outer boundary
		for (auto p = pwh->holes().begin(); p != pwh->holes().end(); ++p) {
			if (p != h_ad && p != h_bc) {
				new_holes.push_back(&*p);
			}
		}
		new_poly.reverse_orientation();
		new_outer = &new_poly;
	}

	struct hole_deref : public std::unary_function<polygon_2 *, const polygon_2 &> {
		const polygon_2 & operator () (polygon_2 * p) const { return *p; }
	};

	*pwh = polygon_with_holes_2(*new_outer, new_holes | boost::adaptors::indirected);

	return res;
}

} // namespace

void polygon_with_holes_2::cleanup() {
	bool cleanup_ok = geometry_common::cleanup_loop(&m_outer, g_opts.equality_tolerance);
	if (!cleanup_ok && FLAGGED(SBT_EXPENSIVE_CHECKS)) {
		ERROR_MSG("Couldn't clean up a pwh_2's outer boundary:\n");
		PRINT_POLYGON(m_outer);
		abort();
	}
	boost::for_each(m_holes, [](polygon_2 & hole) {
		bool cleanup_ok = geometry_common::cleanup_loop(&hole, g_opts.equality_tolerance);
		if (!cleanup_ok && FLAGGED(SBT_EXPENSIVE_CHECKS)) {
			ERROR_MSG("Couldn't clean up a pwh_2 hole:\n");
			PRINT_POLYGON(hole);
			abort();
		}
	});
}

void polygon_with_holes_2::reverse() {
	m_outer.reverse_orientation();
	boost::for_each(m_holes, [](polygon_2 & poly) {
		poly.reverse_orientation();
	});
}

bool polygon_with_holes_2::is_axis_aligned() const {
	return 
		geometry_common::is_axis_aligned(m_outer) &&
		boost::find_if(m_holes, [](const polygon_2 & hole) { return !geometry_common::is_axis_aligned(hole); }) == m_holes.end();
}

std::vector<polygon_2> polygon_with_holes_2::all_polygons() const { 
	std::vector<polygon_2> res(m_holes.begin(), m_holes.end());
	res.push_back(m_outer);
	return res;
}

std::vector<polygon_2> polygon_with_holes_2::to_simple_polygons() const {
	std::vector<polygon_2> res;
	polygon_with_holes_2 copy(*this);
	while (copy.holes().size() > 0) {
		res.push_back(extract_section(&copy));
	}
	res.push_back(copy.outer());
	return res;
}