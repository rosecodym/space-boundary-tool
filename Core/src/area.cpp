#include "precompiled.h"

#include "equality_context.h"
#include "geometry_common.h"
#include "printing-macros.h"

#include "area.h"

namespace geometry_2d {

area::area(const std::vector<std::vector<point_2>> & loops) : use_nef(loops.size() > 1) {
	if (use_nef) {
		nef_rep = wrapped_nef_polygon(loops);
	}
	else {
		simple_rep = polygon_2(loops.front().begin(), loops.front().end());
		ensure_counterclockwise();
		validate();
	}
}

area::area(const std::vector<polygon_2> & loops) : use_nef(loops.size() > 1) {
	if (use_nef) {
		nef_rep = wrapped_nef_polygon(loops);
	}
	else {
		simple_rep = loops.front();
		ensure_counterclockwise();
		validate();
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

void area::validate() const {
	if (FLAGGED(SBT_EXPENSIVE_CHECKS) && !use_nef && !simple_rep.is_simple()) {
		ERROR_MSG("Error: non-simple area representation.\n");
		std::for_each(simple_rep.vertices_begin(), simple_rep.vertices_end(), [](const point_2 & p) {
			ERROR_MSG("(%f, %f)\n", CGAL::to_double(p.x()), CGAL::to_double(p.y()));
		});
		exit(1);
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

area & area::operator *= (const area & other) {
	*this = *this * other;
	return *this;
}

area & area::operator -= (const area & other) {
	if (is_empty() || other.is_empty()) { return *this; }
	if (*this == other) {
		clear();
	}
	else {
		promote_both(*this, other);
		nef_rep -= other.nef_rep;
	}
	return *this;
}

area & area::operator ^= (const area & other) {
	if (other.is_empty()) { return *this; }
	if (is_empty()) { return *this = other; }
	if (*this == other) { 
		clear(); 
	}
	else {
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

area operator - (const area & a, const area & b) {
	if (a.is_empty()) { return area(); }
	if (b.is_empty()) { return a; }
	if (!a.use_nef && !b.use_nef && a.simple_rep == b.simple_rep) { return area(); }
	area::promote_both(a, b);
	return area(a.nef_rep - b.nef_rep);
}

area operator * (const area & a, const area & b) {
	if (FLAGGED(SBT_VERBOSE_BLOCKS) || FLAGGED(SBT_VERBOSE_STACKS)) {
		NOTIFY_MSG("Entered area intersection. Intersecting\n");
		a.print();
		NOTIFY_MSG("with\n");
		b.print();
	}
	if (a.is_empty() || b.is_empty() || !CGAL::do_overlap(a.bbox(), b.bbox())) { return area(); }
	if (!a.use_nef && !b.use_nef && a.simple_rep == b.simple_rep) { return a; }
	area::promote_both(a, b);
	PRINT_2D_OPERATIONS("Polygons promoted.\n");
	return area(a.nef_rep * b.nef_rep);
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

} // namespace geometry_2d