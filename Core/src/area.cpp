#include "precompiled.h"

#include "area.h"

#include "equality_context.h"
#include "geometry_common.h"
#include "stringification.h"

namespace geometry_2d {

area::area(const std::vector<std::vector<point_2>> & loops) : use_nef(loops.size() > 1) {
	if (use_nef) {
		nef_rep = wrapped_nef_polygon(loops);
	}
	else {
		simple_rep = polygon_2(loops.front().begin(), loops.front().end());
		ensure_counterclockwise();
	}
}

area::area(const std::vector<polygon_2> & loops) : use_nef(loops.size() > 1) {
	if (use_nef) {
		nef_rep = wrapped_nef_polygon(loops);
	}
	else {
		simple_rep = loops.front();
		ensure_counterclockwise();
	}
}

area & area::operator = (const area & src) {
	if (&src != this) {
		use_nef = src.use_nef;
		if (!use_nef) { simple_rep = src.simple_rep; }
		else { nef_rep = src.nef_rep; }
	}
	return *this;
}

area & area::operator = (area && src) {
	if (&src != this) {
		use_nef = src.use_nef;
		if (!use_nef) { simple_rep = std::move(src.simple_rep); }
		else { nef_rep = std::move(src.nef_rep); }
	}
	return *this;
}

void area::promote() const {
	if (!use_nef) {
		use_nef = true;
		nef_rep = wrapped_nef_polygon(simple_rep);
	}
}

void area::ensure_counterclockwise() {
	if (!use_nef && simple_rep.is_clockwise_oriented()) {
		simple_rep.reverse_orientation();
	}
}

boost::optional<polygon_2> area::outer_bound() const {
	auto pwhs = to_pwhs();
	return pwhs.size() == 1 ? pwhs.front().outer() : boost::optional<polygon_2>();
}

std::vector<polygon_2> area::to_simple_convex_pieces() const {
	if (!use_nef && simple_rep.is_convex()) { return std::vector<polygon_2>(1, simple_rep); }
	else {
		promote();
		return nef_rep.to_simple_convex_pieces();
	}
}

bool area::any_points_satisfy_predicate(const std::function<bool(point_2)> & pred) const {
	if (!use_nef) {
		return std::find_if(simple_rep.vertices_begin(), simple_rep.vertices_end(), pred) != simple_rep.vertices_end();
	}
	else {
		return nef_rep.any_points_satisfy_predicate(pred);
	}
}

std::string area::to_string() const {
	if (use_nef) { return nef_rep.to_string(); }
	else { return reporting::to_string(simple_rep); }
}

area & area::operator += (const area & other) {
	if (is_empty()) { *this = other; }
	else if (!other.is_empty()) {
		promote_both(*this, other);
		nef_rep += other.nef_rep;
	}
	return *this;
}

area & area::operator -= (const area & other) {
	if (!is_empty() && !other.is_empty()) {
		promote_both(*this, other);
		nef_rep -= other.nef_rep;
	}
	return *this;
}

area & area::operator *= (const area & other) {
	if (is_empty() || other.is_empty()) { clear(); }
	else {
		promote_both(*this, other);
		nef_rep *= other.nef_rep;
	}
	return *this;
}

area & area::operator ^= (const area & other) {
	if (is_empty()) { *this = other; }
	else if (!other.is_empty()) {
		promote_both(*this, other);
		nef_rep ^= other.nef_rep;
	}
	return *this;
}

bool area::do_intersect(const area & a, const area & b) {
	if (a.is_empty() || b.is_empty() || !CGAL::do_overlap(a.bbox(), b.bbox())) { return false; }
	if (!a.use_nef && !b.use_nef && a.simple_rep == b.simple_rep) { return true; }
	promote_both(a, b);
	return wrapped_nef_polygon::do_intersect(a.nef_rep, b.nef_rep);
}

bool operator == (const area & a, const area & b) {
	if (a.is_empty() && b.is_empty()) { return true; }
	if (!a.use_nef && !b.use_nef) { return a.simple_rep == b.simple_rep; }
	area::promote_both(a, b);
	return a.nef_rep == b.nef_rep;
}

bool operator >= (const area & a, const area & b) {
	if (b.is_empty()) { return true; }
	if (a.is_empty()) { return false; }
	area::promote_both(a, b);
	return a.nef_rep >= b.nef_rep;
}

void area::clear() {
	simple_rep.clear();
	nef_rep.clear();
	use_nef = false;
}

std::vector<polygon_with_holes_2> area::to_pwhs() const {
	if (!use_nef) { 
		return std::vector<polygon_with_holes_2>(1, polygon_with_holes_2(std::vector<point_2>(simple_rep.vertices_begin(), simple_rep.vertices_end()), std::vector<std::vector<point_2>>()));
	}
	else {
		return nef_rep.to_pwhs();
	}
}

NT area::regular_area() const {
	if (use_nef) { return nef_rep.outer_regular_area(); }
	else { return geometry_common::regular_area(simple_rep); }
}

area area::snap(equality_context * c) const {
	if (use_nef) {
		return area(nef_rep.update_all([c](const point_2 & p) {
			return c->snap(p);
		}));
	}
	else { return area(c->snap(simple_rep)); }
}

} // namespace geometry_2d