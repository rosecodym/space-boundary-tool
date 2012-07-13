#include "precompiled.h"

#include "equality_context.h"
#include "geometry_common.h"
#include "misc-util.h"
#include "operations.h"
#include "polygon_with_holes_2.h"
#include "printing-macros.h"
#include "printing-util.h"
#include "sbt-core.h"

#include "wrapped_nef_polygon.h"

// HERE BE DRAGONS

#define PRINT_GEOM(...) \
	do { \
		if (g_opts.flags & SBT_VERBOSE_GEOMETRY) { \
			NOTIFY_MSG( __VA_ARGS__); \
		} \
	} \
	while (false);

extern sb_calculation_options g_opts;

namespace geometry_2d {

namespace {

template <class OutT, class InT>
OutT switch_point_kernel(const InT & p) {
	return OutT(p.x(), p.y());
}

template<>
espoint_2 switch_point_kernel(const epoint_2 & in) {
	return eK().standard_point(in);
}

struct extender : public std::unary_function<point_2, espoint_2> {
	espoint_2 operator () (const point_2 & p) const { return switch_point_kernel<espoint_2, point_2>(p); }
};

template <class PointT>
PointT flatten(const PointT & p) {
	return PointT(CGAL::to_double(p.x()), CGAL::to_double(p.y()));
}

nef_polygon_2 create_nef_polygon(polygon_2 poly) {
	if (g_opts.flags & SBT_VERBOSE_GEOMETRY) {
		NOTIFY_MSG( "[creating nef polygon from poly]\n[uncleaned poly follows]\n");
		util::printing::print_polygon(g_opts.notify_func, poly);
	}
	if (!geometry_common::cleanup_loop(&poly, g_opts.equality_tolerance)) {
		ERROR_MSG("[Aborting - couldn't clean up a polygon in order to construct a nef polygon.]\n");
		util::printing::print_polygon(g_opts.error_func, poly);
		throw core_exception(SBT_ASSERTION_FAILED);
	}
	if (g_opts.flags & SBT_VERBOSE_GEOMETRY) {
		NOTIFY_MSG( "[cleaned poly follows]\n");
		util::printing::print_polygon(g_opts.notify_func, poly);
	}
	SBT_ASSERT(poly.is_simple(), "[Aborting - tried to create a nef polygon out of non-simple polygon.]\n");
	std::vector<espoint_2> extended;
	std::transform(poly.vertices_begin(), poly.vertices_end(), std::back_inserter(extended), [](const point_2 & p) {
		return switch_point_kernel<espoint_2, point_2>(p);
	});
	PRINT_2D_OPERATIONS("Points kernel switched to extended. Creating nef polygon.\n");
	nef_polygon_2 res = poly.is_counterclockwise_oriented() ?
		nef_polygon_2(extended.begin(), extended.end(), nef_polygon_2::EXCLUDED) :
		nef_polygon_2(extended.rbegin(), extended.rend(), nef_polygon_2::EXCLUDED);
	PRINT_2D_OPERATIONS("Nef polygon created.\n");
	return res;
}

nef_polygon_2 create_nef_polygon(const std::vector<point_2> & loop) {
	return create_nef_polygon(polygon_2(loop.begin(), loop.end()));
}

bool matches(const epoint_2 & epoint, const point_2 & p, std::shared_ptr<equality_context> c) {
	espoint_2 espoint = eK().standard_point(epoint);
	NT squared_distance = CGAL::square(espoint.x() - p.x()) + CGAL::square(espoint.y() - p.y());
	return c->is_zero_squared(squared_distance);
}

bool remove_duplicate_points(nef_polygon_2 * n) {
	const nef_polygon_2 & nef = *n;

	if (nef.is_empty()) {
		return false;
	}

	std::vector<std::deque<espoint_2>> loops;
	bool changed = false;

	nef_polygon_2::Explorer e = nef.explorer();
	for (auto f = e.faces_begin(); f != e.faces_end(); ++f) {
		if (f->mark()) {
			auto v_iter = e.face_cycle(f);
			loops.push_back(std::deque<espoint_2>());
			auto p = v_iter;
			auto end = p;
			CGAL_For_all(p, end) {
				if (e.is_standard(p->vertex())) {
					espoint_2 thispoint = eK().standard_point(p->vertex()->point());
					if (loops.back().empty() || !equality_context::are_effectively_same(thispoint, loops.back().back(), g_opts.equality_tolerance)) {
						loops.back().push_back(thispoint);
					}
					else {
						changed = true;
					}
				}
			}
			if (equality_context::are_effectively_same(loops.back().back(), loops.back().front(), g_opts.equality_tolerance)) {
				loops.pop_back();
				changed = true;
			}
			if (loops.back().empty()) {
				loops.pop_back();
				continue;
			}
			for (auto h = e.holes_begin(f); h != e.holes_end(f); ++h) {
				nef_polygon_2::Explorer::Halfedge_around_face_const_circulator v_iter = h;
				loops.push_back(std::deque<espoint_2>());
				auto p = v_iter;
				auto end = p;
				CGAL_For_all(p, end) {
					if (e.is_standard(p->vertex())) {
						espoint_2 thispoint = eK().standard_point(p->vertex()->point());
						if (loops.back().empty() || !equality_context::are_effectively_same(thispoint, loops.back().back(), g_opts.equality_tolerance)) {
							loops.back().push_front(thispoint);
						}
						else {
							changed = true;
						}
					}
				}
				if (equality_context::are_effectively_same(loops.back().back(), loops.back().front(), g_opts.equality_tolerance)) {
					loops.pop_back();
					changed = true;
				}
				if (loops.back().empty()) { 
					loops.pop_back(); 
				}
			}
		}
	}

	if (changed) {
		n->clear();
		std::for_each(loops.begin(), loops.end(), [n](const std::deque<espoint_2> & loop) { 
			*n ^= nef_polygon_2(loop.begin(), loop.end(), nef_polygon_2::EXCLUDED);
		});
		*n = n->interior();
	}

	return changed;
}

template <class OutputIterator>
void get_all_points(const nef_polygon_2 & nef, OutputIterator oi) {
	nef_polygon_2::Explorer e = nef.explorer();
	for (auto v = e.vertices_begin(); v != e.vertices_end(); ++v) {
		if (e.is_standard(v)) {
			*oi++ = e.point(v);
		}
	}
}

template <class OutputIterator>
void get_all_lines(const nef_polygon_2 & nef, OutputIterator oi) {
	nef_polygon_2::Explorer e = nef.explorer();
	for (auto he = e.halfedges_begin(); he != e.halfedges_end(); ++he) {
		*oi++ = esegment_2(he->vertex()->point(), he->opposite()->vertex()->point()).supporting_line();
	}
}

bool point_matches(const espoint_2 & espoint, const point_2 & p) {
	NT squared_distance = CGAL::square(espoint.x() - p.x()) + CGAL::square(espoint.y() - p.y());
	return equality_context::is_zero_squared(squared_distance, g_opts.equality_tolerance);
}

template <class LineIter>
LineIter find_close_line(const epoint_2 & epoint, LineIter lines_begin, LineIter lines_end) {
	return std::find_if(lines_begin, lines_end, [&epoint](const eline_2 & line) { 
		return equality_context::is_zero_squared(CGAL::to_double(CGAL::squared_distance(flatten(epoint), line)), g_opts.equality_tolerance);
	});
}

template <class PointInIter, class OutIter>
bool check_for_point_match(const epoint_2 & check_point, PointInIter existing_begin, PointInIter existing_end, bool * changed, OutIter & oi) {
	espoint_2 espoint = eK().standard_point(check_point);
	auto match = std::find_if(existing_begin, existing_end, [&espoint](const point_2 & existing) { return point_matches(espoint, existing); });
	if (match != existing_end) {
		if (match->x() == espoint.x() && match->y() == espoint.y()) {
			*oi++ = espoint;
			PRINT_GEOM("[snapping point (%f, %f) (exact point match)]\n", CGAL::to_double(espoint.x()), CGAL::to_double(espoint.y()));
		}
		else {
			*oi++ = espoint_2(match->x(), match->y());
			PRINT_GEOM("[snapping point (%f, %f) (inexact point match)]\n", CGAL::to_double(match->x()), CGAL::to_double(match->y()));
			*changed = true;
		}
		return true;
	}
	return false;
}

template <class LineInIter, class OutIter>
bool check_for_line_match(const epoint_2 & check_point, LineInIter lines_begin, LineInIter lines_end, bool * changed, OutIter & oi) {
	auto first_match = find_close_line(check_point, lines_begin, lines_end);

	if (first_match == lines_end) {
		return false;
	}

	auto second_match = first_match;
	do {
		++second_match;
		second_match = find_close_line(check_point, second_match, lines_end);
	}
	while (second_match != lines_end && CGAL::parallel(*first_match, *second_match));

	if (second_match == lines_end) {
		if (first_match->has_on(check_point)) {
			*oi++ = eK().standard_point(check_point);
		}
		else {
			*oi++ = eK().standard_point(first_match->projection(check_point));
			PRINT_GEOM("[snapping point (%f, %f) (projection to <%f, %f>)]\n", 
				CGAL::to_double(check_point.x()), 
				CGAL::to_double(check_point.y()),
				CGAL::to_double(first_match->direction().dx()),
				CGAL::to_double(first_match->direction().dy()));
			*changed = true;
		}
		return true;
	}

	*oi++ = eK().standard_point(CGAL::object_cast<epoint_2>(CGAL::intersection(*first_match, *second_match)));
	PRINT_GEOM("[snapping point (%f, %f) (line intersection between <%f, %f> and <%f, %f>)]\n", 
		CGAL::to_double(check_point.x()), 
		CGAL::to_double(check_point.y()),
		CGAL::to_double(first_match->direction().dx()),
		CGAL::to_double(first_match->direction().dy()),
		CGAL::to_double(second_match->direction().dx()),
		CGAL::to_double(second_match->direction().dy()));
	*changed = true;
	return true;
}

template <class VertexIterable, class PointInIter, class LineInIter, class OutIter>
void process_loop(
	const nef_polygon_2::Explorer & e, 
	VertexIterable v_iter, 
	PointInIter existing_points_begin, 
	PointInIter existing_points_end, 
	LineInIter existing_lines_begin, 
	LineInIter existing_lines_end, 
	bool * changed,
	OutIter oi) 
{
	PRINT_GEOM("[processing loop]\n");
	auto p = v_iter;
	auto end = p;
	CGAL_For_all(p, end) {
		if (e.is_standard(p->vertex())) {
			epoint_2 thispoint = p->vertex()->point();
			PRINT_GEOM("[checking point (%f, %f)]\n", CGAL::to_double(thispoint.x()), CGAL::to_double(thispoint.y()));
			if (!check_for_point_match(thispoint, existing_points_begin, existing_points_end, changed, oi) &&
				!check_for_line_match(thispoint, existing_lines_begin, existing_lines_end, changed, oi))
			{
				*oi++ = eK().standard_point(thispoint);
			}
		}
	}
}

void snap_nef_polygon_to(nef_polygon_2 * from, const nef_polygon_2 & to) {
	if (from->is_empty()) {
		return;
	}

	IF_FLAGGED(SBT_VERBOSE_GEOMETRY) {
		NOTIFY_MSG( "[snapping nef polygon]\n");
		util::printing::print_nef_polygon(g_opts.notify_func, *from);
		NOTIFY_MSG( "[to]\n");
		util::printing::print_nef_polygon(g_opts.notify_func, to);
	}

	std::vector<point_2> all_points;
	std::vector<eline_2> all_lines;
	get_all_points(to, std::back_inserter(all_points));
	PRINT_GEOM("[got all points from target nef]\n");
	get_all_lines(to, std::back_inserter(all_lines));
	PRINT_GEOM("[got all lines from target nef]\n");

	std::vector<std::deque<espoint_2>> loops;
	bool changed = false;

	nef_polygon_2::Explorer e = from->explorer();
	for (auto f = e.faces_begin(); f != e.faces_end(); ++f) {
		if (f->mark()) {
			auto v_iter = e.face_cycle(f);
			loops.push_back(std::deque<espoint_2>());
			process_loop(e, v_iter, all_points.begin(), all_points.end(), all_lines.begin(), all_lines.end(), &changed, std::back_inserter(loops.back()));
			if (loops.back().empty()) {
				loops.pop_back();
				continue;
			}
			PRINT_GEOM("[loaded face loop]\n");
			for (auto h = e.holes_begin(f); h != e.holes_end(f); ++h) {
				nef_polygon_2::Explorer::Halfedge_around_face_const_circulator v_iter = h;
				loops.push_back(std::deque<espoint_2>());
				process_loop(e, v_iter, all_points.begin(), all_points.end(), all_lines.begin(), all_lines.end(), &changed, std::front_inserter(loops.back()));
				if (loops.back().empty()) { loops.pop_back(); }
				else { PRINT_GEOM("loaded hole loop]\n"); }
			}
		}
	}

	if (changed) {
		PRINT_GEOM("[polygon changed - modifying (%u loops)]\n", loops.size());
		nef_polygon_2 orig = *from;
		from->clear();
		std::for_each(loops.begin(), loops.end(), [from, &to, &orig](const std::deque<espoint_2> & loop) { 
			IF_FLAGGED(SBT_VERBOSE_GEOMETRY) {
				NOTIFY_MSG( "[loop:]\n");
				std::for_each(loop.begin(), loop.end(), [](const espoint_2 & p) { NOTIFY_MSG( "%f\t%f\n", CGAL::to_double(p.x()), CGAL::to_double(p.y())); });
			}
			std::vector<point_2> polypoints;
			boost::transform(loop, std::back_inserter(polypoints), [](const espoint_2 & p) { return point_2(p.x(), p.y()); });
			if (polygon_2(polypoints.begin(), polypoints.end()).is_simple()) {
				nef_polygon_2 nef(loop.begin(), loop.end(), nef_polygon_2::EXCLUDED); 
				*from ^= nef;
			}
		});
	}

	// this needs to happen whether there was change or not because: i have no idea
	// this is all HERE BE DRAGONS code anyway
	*from = from->interior();

	PRINT_GEOM("[nef polygon snap complete]\n");
}

} // namespace

void wrapped_nef_polygon::snap_to(const wrapped_nef_polygon & other) {
	snap_nef_polygon_to(wrapped.get(), *other.wrapped);
	SBT_EXPENSIVE_ASSERT(is_valid(), "[Aborting - a nef polygon snap made it invalid.]\n");
}

void wrapped_nef_polygon::print_with(void (*msg_func)(char *)) const {
	if (is_empty()) {
		msg_func("[-empty polygon-]\n");
	}
	else {
		util::printing::print_nef_polygon(msg_func, *wrapped);
	}
}

wrapped_nef_polygon::wrapped_nef_polygon(const nef_polygon_2 & nef, bool aligned)
	: wrapped(new nef_polygon_2(nef.interior())), 
	m_is_axis_aligned(aligned) 
{ 
	remove_duplicate_points(wrapped.get());
	SBT_EXPENSIVE_ASSERT(wrapped->is_empty() == wrapped->interior().is_empty(), "[Aborting - created a non-regularized nef polygon (explicit alignment constructor).]\n");
}

wrapped_nef_polygon::wrapped_nef_polygon(const polygon_2 & poly)
	: wrapped(new nef_polygon_2(create_nef_polygon(poly))), m_is_axis_aligned(util::cgal::is_axis_aligned(poly))
{
	PRINT_2D_OPERATIONS("Entered wrapped_nef_polygon ctor body (from polygon_2).\n");
}

wrapped_nef_polygon::wrapped_nef_polygon(const polygon_with_holes_2 & pwh) 
	: wrapped(new nef_polygon_2(create_nef_polygon(pwh.outer()))), m_is_axis_aligned(pwh.is_axis_aligned())
{
	boost::for_each(pwh.holes(), [this](const polygon_2 & hole) { *this -= wrapped_nef_polygon(hole); });

	SBT_EXPENSIVE_ASSERT(wrapped->is_empty() == wrapped->interior().is_empty(), "[Aborting - created a non-regularized nef polygon out of a pwh.]\n");
}

wrapped_nef_polygon::wrapped_nef_polygon(const wrapped_nef_polygon & src, equality_context * recontextualization_c) : m_is_axis_aligned(true) {
	nef_polygon_2::Explorer e = src.wrapped->explorer();
	std::vector<polygon_2> loops;
	for (auto f = e.faces_begin(); f != e.faces_end(); ++f) {
		if (f->mark()) {
			auto pwh_maybe = create_pwh_2(e, f); // if the face is too small there won't be anything
			if (pwh_maybe) {
				boost::transform(pwh_maybe->all_polygons(), std::back_inserter(loops), [recontextualization_c](const polygon_2 & loop) {
					return recontextualization_c->snap(loop);
				});
			}
		}
	}
	*this = wrapped_nef_polygon(loops);
}

wrapped_nef_polygon & wrapped_nef_polygon::operator -= (const wrapped_nef_polygon & other) {
	if (!m_is_axis_aligned || !other.m_is_axis_aligned) {
		wrapped_nef_polygon other_snapped(other);
		other_snapped.snap_to(*this);
		PRINT_2D_OPERATIONS("Performing nef_polygon subtraction.\n");
		*wrapped -= other_snapped.wrapped->interior();
		PRINT_2D_OPERATIONS("Subtraction complete.\n");
		m_is_axis_aligned = false;
	}
	else {
		*wrapped -= other.wrapped->interior();
	}
	*wrapped = wrapped->interior();
	IF_FLAGGED(SBT_EXPENSIVE_CHECKS) {
		if (!is_valid()) {
			ERROR_MSG("[Aborting - wrapped_nef_polygon::operator -= produced an invalid polygon. Here it is:]\n");
			print_with(g_opts.error_func);
			throw core_exception(SBT_ASSERTION_FAILED);
		}
	}
	return *this;
}

wrapped_nef_polygon & wrapped_nef_polygon::operator ^= (const wrapped_nef_polygon & other) {
	if (!m_is_axis_aligned || !other.m_is_axis_aligned) {
		wrapped_nef_polygon other_snapped(other);
		other_snapped.snap_to(*this);
		PRINT_2D_OPERATIONS("Performing nef_polygon symmetric difference.\n");
		*wrapped ^= other_snapped.wrapped->interior();
		PRINT_2D_OPERATIONS("Symmetric difference complete.\n");
		m_is_axis_aligned = false;
	}
	else {
		*wrapped -= other.wrapped->interior();
	}
	*wrapped = wrapped->interior();
	SBT_EXPENSIVE_ASSERT(is_valid(), "[Aborting - wrapped_nef_polygon::operator ^= produced an invalid polygon.]\n");
	return *this;
}

bool wrapped_nef_polygon::is_empty() const {
	if (!wrapped || wrapped->is_empty()) {
		return true;
	}
	nef_polygon_2::Explorer e = wrapped->explorer();
	for (auto f = e.faces_begin(); f != e.faces_end(); ++f) {
		if (f->mark() /*|| e.holes_begin(f) != e.holes_end(f)*/) {
			return false;
		}
	}
	return true;
}

bbox_2 wrapped_nef_polygon::bbox() const {
	boost::optional<bbox_2> res;
	nef_polygon_2::Explorer e = wrapped->explorer();
	for (auto v = e.vertices_begin(); v != e.vertices_end(); ++v) {
		if (e.is_standard(v)) {
			if (res) {
				res = *res + v->point().bbox();
			}
			else {
				res = v->point().bbox();
			}
		}
	}
	return *res;
}

std::vector<polygon_with_holes_2> wrapped_nef_polygon::to_pwhs() const {
	std::vector<polygon_with_holes_2> res;
	nef_polygon_2::Explorer e = wrapped->explorer();
	for (auto f = e.faces_begin(); f != e.faces_end(); ++f) {
		if (f->mark()) {
			auto pwh_maybe = create_pwh_2(e, f); // if the face is too small there won't be anything
			if (pwh_maybe) {
				res.push_back(*pwh_maybe);
			}
		}
	}
	return res;
}

polygon_2 wrapped_nef_polygon::outer() const {
	polygon_2 outer;
	nef_polygon_2::Explorer e = wrapped->explorer();
	for (auto f = e.faces_begin(); f != e.faces_end(); ++f) {
		if (f->mark()) {
			auto p = e.face_cycle(f);
			do {
				if (e.is_standard(p->vertex())) {
					outer.push_back(point_2(p->vertex()->point().x().content(), p->vertex()->point().y().content()));
				}
				++p;
			}
			while (p != e.face_cycle(f));
			break;
		}
	}

	SBT_ASSERT(!outer.is_empty(), "[Aborting - a wrapped_nef_polygon face had no standard points on its outer boundary.]\n");

	return outer;
}

boost::optional<polygon_with_holes_2> wrapped_nef_polygon::create_pwh_2(const nef_polygon_2::Explorer & e, nef_polygon_2::Explorer::Face_const_handle f) {
	polygon_2 outer;
	std::vector<polygon_2> holes;
	auto p = e.face_cycle(f);
	do {
		if (e.is_standard(p->vertex())) {
			outer.push_back(point_2(CGAL::to_double(p->vertex()->point().x()), CGAL::to_double(p->vertex()->point().y())));
		}
		++p;
	}
	while (p != e.face_cycle(f));

	if (!geometry_common::cleanup_loop(&outer, g_opts.equality_tolerance)) {
		return boost::optional<polygon_with_holes_2>();
	}

	for (auto h = e.holes_begin(f); h != e.holes_end(f); ++h) {
		nef_polygon_2::Explorer::Halfedge_around_face_const_circulator curr = h;
		decltype(curr) end = curr;
		holes.push_back(polygon_2());
		CGAL_For_all(curr, end) {
			if (e.is_standard(curr->vertex())) {
				holes.back().push_back(point_2(CGAL::to_double(curr->vertex()->point().x()), CGAL::to_double(curr->vertex()->point().y())));
			}
			++p;
		}
		if (!geometry_common::cleanup_loop(&holes.back(), g_opts.equality_tolerance)) {
			holes.pop_back();
		}
	}

	return polygon_with_holes_2(outer, holes);
}

polygon_2 wrapped_nef_polygon::to_single_polygon() const {
	std::vector<polygon_2> polys;
	to_simple_polygons(std::back_inserter(polys));
	if (polys.size() != 1) {
		ERROR_MSG("[Aborting - tried to get a single polygon out of a nef polygon, but it didn't work (%u polys).]\n", polys.size());
		throw core_exception(SBT_ASSERTION_FAILED);
	}
	return polys.front();
}

bool wrapped_nef_polygon::is_valid() const {
	if (wrapped->is_empty()) {
		return true;
	}
	else if (*wrapped == wrapped->interior()) {
		std::vector<espoint_2> all_points;
		nef_polygon_2::Explorer e = wrapped->explorer();
		for (auto v = e.vertices_begin(); v != e.vertices_end(); ++v) {
			if (e.is_standard(v)) {
				all_points.push_back(switch_point_kernel<espoint_2, epoint_2>(v->point()));
			}
		}
		bool res = !equality_context::is_zero_squared(util::cgal::smallest_squared_distance(all_points.begin(), all_points.end()), g_opts.equality_tolerance);
		if (!res) {
			ERROR_MSG("[Invalid polygon - some points were too close.]\n");
		}
		return res;
	}
	else {
		ERROR_MSG("[Invalid polygon - not an interior.]\n");
		return false;
	}
}

bool wrapped_nef_polygon::any_points_satisfy_predicate(const std::function<bool(point_2)> & pred) const {
	nef_polygon_2::Explorer e = wrapped->explorer();
	for (auto v = e.vertices_begin(); v != e.vertices_end(); ++v) {
		if (e.is_standard(v) && pred(point_2(eK().standard_point(v->point()).x(), eK().standard_point(v->point()).y()))) {
			return true;
		}
	}
	return false;
}

bool wrapped_nef_polygon::do_intersect(const wrapped_nef_polygon & lhs, const wrapped_nef_polygon & rhs) {
	if (!lhs.m_is_axis_aligned && !rhs.m_is_axis_aligned) {
		wrapped_nef_polygon r_snapped(rhs);
		r_snapped.snap_to(lhs);
		return !(lhs * r_snapped).is_empty();
	}
	else {
		return !(lhs * rhs).is_empty();
	}
}

wrapped_nef_polygon operator - (const wrapped_nef_polygon & lhs, const wrapped_nef_polygon & rhs) {
	if (!lhs.m_is_axis_aligned || !rhs.m_is_axis_aligned) {
		wrapped_nef_polygon r_snapped(rhs);
		r_snapped.snap_to(lhs);
		return wrapped_nef_polygon((*lhs.wrapped - *r_snapped.wrapped).interior(), false);
	}
	else {
		return wrapped_nef_polygon((*lhs.wrapped - *rhs.wrapped).interior(), true);
	}
}

wrapped_nef_polygon operator * (const wrapped_nef_polygon & lhs, const wrapped_nef_polygon & rhs) {
	PRINT_GEOM("Entered wrapped_nef_polygon::operator *.\n");
	if (!lhs.m_is_axis_aligned || !rhs.m_is_axis_aligned) {
		PRINT_GEOM("Polygons are not axis-aligned.\n");
		wrapped_nef_polygon r_snapped(rhs);
		r_snapped.snap_to(lhs);
		return wrapped_nef_polygon((*lhs.wrapped * *r_snapped.wrapped).interior(), false);
	}
	else {
		return wrapped_nef_polygon((*lhs.wrapped * *rhs.wrapped).interior(), true);
	}
}

} // namespace geometry_2d