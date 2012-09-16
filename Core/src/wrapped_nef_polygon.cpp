#include "precompiled.h"

#include "equality_context.h"
#include "geometry_common.h"
#include "nef_polygon_face.h"
#include "nef_polygon_util.h"

#include "wrapped_nef_polygon.h"

namespace geometry_2d {

namespace nef_polygons {

std::vector<face> wrapped_nef_polygon::get_faces() const {
	current_explorer = wrapped->explorer();
	std::vector<face> res;
	for (auto f = current_explorer->faces_begin(); f != current_explorer->faces_end(); ++f) {
		if (f->mark()) {
			res.push_back(face(*current_explorer, f));
		}
	}
	return res;
}

bool wrapped_nef_polygon::any_points_satisfy_predicate(const std::function<bool(point_2)> & pred) const {
	auto e = wrapped->explorer();
	for (auto v = e.vertices_begin(); v != e.vertices_end(); ++v) {
		if (v->mark() && e.is_standard(v) && pred(e.point(v))) {
			return true;
		}
	}
	return false;
}

bbox_2 wrapped_nef_polygon::bbox() const {
	boost::optional<bbox_2> res;
	auto e = wrapped->explorer();
	for (auto v = e.vertices_begin(); v != e.vertices_end(); ++v) {
		if (v->mark() && e.is_standard(v)) {
			if (!res) { res = e.point(v).bbox(); }
			else { res = *res + e.point(v).bbox(); }
		}
	}
	return res ? *res : bbox_2(0, 0, 0, 0);
}

bool wrapped_nef_polygon::is_valid(double eps) const {
	if (is_empty()) { return true; }
	else if (*wrapped != wrapped->interior()) { return false; }
	auto e = wrapped->explorer();
	std::vector<point_2> points;
	for (auto v = e.vertices_begin(); v != e.vertices_end(); ++v) {
		if (v->mark() && e.is_standard(v)) {
			points.push_back(e.point(v));
		}
	}
	return !equality_context::is_zero_squared(geometry_common::smallest_squared_distance(points.begin(), points.end()), eps);
}

void wrapped_nef_polygon::print_with(const std::function<void(char *)> & func) const {
	boost::for_each(get_faces(), [&func](const face & f) { f.print_with(func); });
}

std::vector<polygon_2> wrapped_nef_polygon::to_simple_convex_pieces() const {
	std::vector<polygon_2> res;
	boost::for_each(get_faces(), [&res](const face & f) { 
		f.to_simple_convex_pieces(std::back_inserter(res)); 
	});
	return res;
}

std::vector<polygon_with_holes_2> wrapped_nef_polygon::to_pwhs() const {
	std::vector<polygon_with_holes_2> res;
	boost::copy(
		get_faces()
		| boost::adaptors::transformed([](const face & f) { return f.to_pwh(); })
		| boost::adaptors::filtered([](const boost::optional<polygon_with_holes_2> & pwh) { return pwh; })
		| boost::adaptors::transformed([](const boost::optional<polygon_with_holes_2> & pwh) { return *pwh; }),
		std::back_inserter(res));
	return res;
}

void wrapped_nef_polygon::clear() {
	wrapped->clear();
	m_is_axis_aligned = true;
}

wrapped_nef_polygon & wrapped_nef_polygon::operator -= (const wrapped_nef_polygon & other) {
	if (!m_is_axis_aligned || !other.m_is_axis_aligned) {
		wrapped_nef_polygon other_snapped(other);
		util::snap(&*other_snapped.wrapped, *wrapped);
		*wrapped -= other_snapped.wrapped->interior();
		m_is_axis_aligned = false;
	}
	else {
		*wrapped -= other.wrapped->interior();
	}
	*wrapped = wrapped->interior();
	return *this;
}

wrapped_nef_polygon & wrapped_nef_polygon::operator ^= (const wrapped_nef_polygon & other) {
	if (!m_is_axis_aligned || !other.m_is_axis_aligned) {
		wrapped_nef_polygon other_snapped(other);
		util::snap(&*other_snapped.wrapped, *wrapped);
		*wrapped ^= other_snapped.wrapped->interior();
		m_is_axis_aligned = false;
	}
	else {
		*wrapped -= other.wrapped->interior();
	}
	*wrapped = wrapped->interior();
	return *this;
}

bool wrapped_nef_polygon::do_intersect(const wrapped_nef_polygon & lhs, const wrapped_nef_polygon & rhs) {
	if (!lhs.m_is_axis_aligned && !rhs.m_is_axis_aligned) {
		wrapped_nef_polygon r_snapped(rhs);
		util::snap(&*r_snapped.wrapped, *lhs.wrapped);
		return !(lhs * r_snapped).is_empty();
	}
	else {
		return !(lhs * rhs).is_empty();
	}
}

wrapped_nef_polygon operator + (const wrapped_nef_polygon & lhs, const wrapped_nef_polygon & rhs) {
	if (!lhs.m_is_axis_aligned || !rhs.m_is_axis_aligned) {
		wrapped_nef_polygon r_snapped(rhs);
		util::snap(&*r_snapped.wrapped, *lhs.wrapped);
		return wrapped_nef_polygon((*lhs.wrapped + *r_snapped.wrapped).interior(), false);
	}
	else {
		return wrapped_nef_polygon((*lhs.wrapped + *rhs.wrapped).interior(), true);
	}
}

wrapped_nef_polygon operator - (const wrapped_nef_polygon & lhs, const wrapped_nef_polygon & rhs) {
	if (!lhs.m_is_axis_aligned || !rhs.m_is_axis_aligned) {
		wrapped_nef_polygon r_snapped(rhs);
		util::snap(&*r_snapped.wrapped, *lhs.wrapped);
		return wrapped_nef_polygon((*lhs.wrapped - *r_snapped.wrapped).interior(), false);
	}
	else {
		return wrapped_nef_polygon((*lhs.wrapped - *rhs.wrapped).interior(), true);
	}
}

wrapped_nef_polygon operator * (const wrapped_nef_polygon & lhs, const wrapped_nef_polygon & rhs) {
	if (!lhs.m_is_axis_aligned || !rhs.m_is_axis_aligned) {
		wrapped_nef_polygon r_snapped(rhs);
		util::snap(&*r_snapped.wrapped, *lhs.wrapped);
		return wrapped_nef_polygon((*lhs.wrapped * *r_snapped.wrapped).interior(), false);
	}
	else {
		return wrapped_nef_polygon((*lhs.wrapped * *rhs.wrapped).interior(), true);
	}
}

} // namespace 

} // namespace