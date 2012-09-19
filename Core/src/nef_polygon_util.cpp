#include "precompiled.h"

#include "cleanup_loop.h"
#include "sbt-core.h"

#include "nef_polygon_util.h"

extern sb_calculation_options g_opts;

namespace geometry_2d {

namespace nef_polygons {

namespace util {

namespace {

template <typename OutputIterator>
void get_all_points(const nef_polygon_2 & nef, OutputIterator oi) {
	auto e = nef.explorer();
	for (auto v = e.vertices_begin(); v != e.vertices_end(); ++v) {
		if (e.is_standard(v)) {
			*oi++ = e.point(v);
		}
	}
}

template <typename OutputIterator>
void get_all_lines(const nef_polygon_2 & nef, OutputIterator oi) {
	auto e = nef.explorer();
	for (auto he = e.halfedges_begin(); he != e.halfedges_end(); ++he) {
		*oi++ = esegment_2(he->vertex()->point(), he->opposite()->vertex()->point()).supporting_line();
	}
}

bool point_matches(const espoint_2 & espoint, const point_2 & p) {
	NT squared_distance = CGAL::square(espoint.x() - p.x()) + CGAL::square(espoint.y() - p.y());
	return equality_context::is_zero_squared(squared_distance, g_opts.equality_tolerance);
}

template <typename LineIter>
LineIter find_close_line(const epoint_2 & epoint, LineIter begin, LineIter end) {
	// this flattening is legacy and i'm terrified to take it out
	epoint_2 flattened(CGAL::to_double(epoint.x()), CGAL::to_double(epoint.y()));
	return std::find_if(begin, end, [&flattened](const eline_2 & line) {
		return equality_context::is_zero_squared(CGAL::to_double(CGAL::squared_distance(flattened, line)), g_opts.equality_tolerance);
	});
}

template <typename PointRange, typename OutputIterator>
bool check_for_point_match(const epoint_2 & check_point, const PointRange & existing_points, bool * changed, OutputIterator oi) {
	espoint_2 espoint = eK().standard_point(check_point);
	auto match = boost::find_if(existing_points, [&espoint](const point_2 & existing) {
		return point_matches(espoint, existing);
	});
	if (match != existing_points.end()) {
		if (match->x() == espoint.x() && match->y() == espoint.y()) {
			*oi++ = espoint;
		}
		else {
			*oi++ = espoint_2(match->x(), match->y());
			*changed = true;
		}
		return true;
	}
	return false;
}

template <typename LineRange, typename OutputIterator>
bool check_for_line_match(const epoint_2 & check_point, const LineRange & lines, bool * changed, OutputIterator oi) {
	auto first_match = find_close_line(check_point, lines.begin(), lines.end());

	if (first_match == lines.end()) { return false; }

	auto second_match = first_match;
	do {
		++second_match;
		second_match = find_close_line(check_point, second_match, lines.end());
	}
	while (second_match != lines.end() && CGAL::parallel(*first_match, *second_match));

	if (second_match == lines.end()) {
		if (first_match->has_on(check_point)) {
			*oi++ = eK().standard_point(check_point);
		}
		else {
			*oi++ = eK().standard_point(first_match->projection(check_point));
			*changed = true;
		}
		return true;
	}

	*oi++ = eK().standard_point(CGAL::object_cast<epoint_2>(CGAL::intersection(*first_match, *second_match)));
	*changed = true;
	return true;
}

template <typename VertexCirculator, typename PointRange, typename LineRange, typename OutputIterator>
void process_loop(
	const nef_polygon_2::Explorer & e, 
	VertexCirculator v_iter, 
	PointRange existing_points,
	LineRange existing_lines,
	bool * changed,
	OutputIterator oi)
{
	auto end = v_iter;
	CGAL_For_all(v_iter, end) {
		if (e.is_standard(v_iter->vertex())) {
			epoint_2 thispoint = v_iter->vertex()->point();
			if (!check_for_point_match(thispoint, existing_points, changed, oi) &&
				!check_for_line_match(thispoint, existing_lines, changed, oi))
			{
				*oi++ = eK().standard_point(thispoint);
			}
		}
	}
}

} // namespace

nef_polygon_2 clean(const nef_polygon_2 & nef) {
	typedef std::vector<espoint_2> loop_t;

	if (nef.is_empty()) { return nef; }

	nef_polygon_2::Explorer e = nef.explorer();
	std::vector<loop_t> loops;

	for (auto f = e.faces_begin(); f != e.faces_end(); ++f) {
		if (f->mark()) {
			loops.push_back(loop_t());
			auto p = e.face_cycle(f);
			auto end = p;
			CGAL_For_all(p, end) {
				if (e.is_standard(p->vertex())) {
					loops.back().push_back(eK().standard_point(p->vertex()->point()));
				}
			}
			if (loops.back().empty()) {
				loops.pop_back();
				continue;
			}
			for (auto h = e.holes_begin(f); h != e.holes_end(f); ++h) {
				loops.push_back(loop_t());
				nef_polygon_2::Explorer::Halfedge_around_face_const_circulator p = h;
				auto end = p;
				if (p != end) {
					CGAL_For_all(p, end) {
						if (e.is_standard(p->vertex())) {
							loops.back().push_back(eK().standard_point(p->vertex()->point()));
						}
					}
				}
			}
		}
	}

	bool changed = false;
	boost::for_each(loops, [&changed](loop_t & loop) { 
		size_t in_size = loop.size();
		if (!geometry_common::cleanup_loop(&loop, g_opts.equality_tolerance)) { loop.clear(); }
		if (loop.size() != in_size) { changed = true; }
	});

	if (!changed) { return nef; }
	else {
		nef_polygon_2 res(nef_polygon_2::EMPTY);
		boost::for_each(loops, [&res](const loop_t & loop) {
			res ^= nef_polygon_2(loop.begin(), loop.end(), nef_polygon_2::EXCLUDED);
		});
		res = res.interior();
		return res;
	}
}

nef_polygon_2 create_nef_polygon(polygon_2 poly) {
	if (!geometry_common::cleanup_loop(&poly, g_opts.equality_tolerance)) {
		return nef_polygon_2::EMPTY;
	}
	std::vector<espoint_2> extended;
	std::transform(poly.vertices_begin(), poly.vertices_end(), std::back_inserter(extended), [](const point_2 & p) { 
		return to_espoint(p); 
	});
	return poly.is_counterclockwise_oriented() ?
		nef_polygon_2(extended.begin(), extended.end(), nef_polygon_2::EXCLUDED) :
		nef_polygon_2(extended.rbegin(), extended.rend(), nef_polygon_2::EXCLUDED);
}

void snap(nef_polygon_2 * from, const nef_polygon_2 & to) {
	if (from->is_empty() || to.is_empty()) { return; }

	std::vector<point_2> all_points;
	std::vector<eline_2> all_lines;
	get_all_points(to, std::back_inserter(all_points));
	get_all_lines(to, std::back_inserter(all_lines));

	std::vector<std::deque<espoint_2>> loops;
	bool changed = false;

	auto e = from->explorer();
	for (auto f = e.faces_begin(); f != e.faces_end(); ++f) {
		if (f->mark()) {
			auto v_iter = e.face_cycle(f);
			loops.push_back(std::deque<espoint_2>());
			process_loop(e, v_iter, all_points, all_lines, &changed, std::back_inserter(loops.back()));
			if (loops.back().empty()) {
				loops.pop_back();
				continue;
			}
			for (auto h = e.holes_begin(f); h != e.holes_end(f); ++h) {
				nef_polygon_2::Explorer::Halfedge_around_face_const_circulator v_iter = h;
				loops.push_back(std::deque<espoint_2>());
				process_loop(e, v_iter, all_points, all_lines, &changed, std::front_inserter(loops.back()));
				if (loops.back().empty()) { 
					loops.pop_back(); 
				}
			}
		}
	}

	if (changed) {
		nef_polygon_2 orig = *from;
		from->clear();
		boost::for_each(loops, [from, &to, &orig](const std::deque<espoint_2> & loop) {
			std::vector<point_2> polypoints;
			boost::transform(loop, std::back_inserter(polypoints), [](const espoint_2 & p) { return point_2(p.x(), p.y()); });
			if (polygon_2(polypoints.begin(), polypoints.end()).is_simple()) {
				nef_polygon_2 nef(loop.begin(), loop.end(), nef_polygon_2::EXCLUDED);
				*from ^= nef;
			}
		});
	}

	// this seems to be necessary even if there isn't a change, but i don't know why
	*from = from->interior();
}

espoint_2 to_espoint(const point_2 & p) { 
	return espoint_2(p.x(), p.y()); 
}

size_t vertex_count(const nef_polygon_2 & nef) {
	// subtract the corners of the infimaximal box
	return nef.explorer().number_of_vertices() - 4; 
}

} // namespace util

} // namespace nef_polygons

} // namespace geometry_2d