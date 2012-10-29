#include "precompiled.h"

#include "equality_context.h"
#include "geometry_common.h"
#include "nef_polygon_face.h"
#include "nef_polygon_util.h"

#include "wrapped_nef_polygon.h"

namespace geometry_2d {

namespace nef_polygons {

wrapped_nef_polygon::wrapped_nef_polygon(const face & f) {
	auto pwh = f.to_pwh();
	if (pwh) {
		*this = wrapped_nef_polygon(pwh->all_polygons());
	}
	else {
		wrapped = std::unique_ptr<nef_polygon_2>(new nef_polygon_2(nef_polygon_2::EMPTY));
		m_is_axis_aligned = true;
	}
}

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
		// don't check marks - points are never marked
		if (e.is_standard(v)) {
			auto p = e.point(v);
			if (pred(point_2(p.x(), p.y()))) {
				return true;
			}
		}
	}
	return false;
}

bbox_2 wrapped_nef_polygon::bbox() const {
	boost::optional<bbox_2> res;
	auto e = wrapped->explorer();
	for (auto v = e.vertices_begin(); v != e.vertices_end(); ++v) {
		// don't check marks - points are never marked
		if (e.is_standard(v)) {
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
		// don't check marks - points are never marked
		if (e.is_standard(v)) {
			auto pt = e.point(v);
			points.push_back(point_2(pt.x(), pt.y()));
		}
	}
	return !equality_context::is_zero_squared(geometry_common::smallest_squared_distance(points.begin(), points.end()), eps);
}

std::string wrapped_nef_polygon::to_string() const {
	std::stringstream ss;
	boost::for_each(get_faces(), [&ss](const face & f) { ss << f.to_string(); });
	return ss.str();
}

std::vector<polygon_2> wrapped_nef_polygon::to_simple_convex_pieces() const {
	std::vector<polygon_2> res;
	auto faces = get_faces();
	if (faces.size() > 1) {
		boost::for_each(faces, [&res](const face & f) {
			boost::copy(wrapped_nef_polygon(f).to_simple_convex_pieces(), std::back_inserter(res));
		});
	}
	else if (faces.size() == 1) {
		boost::optional<polygon_2> outer = faces.front().to_simple_polygon();
		if (outer && outer->is_convex()) { return std::vector<polygon_2>(1, *outer); }

		auto e = wrapped->explorer();
		std::set<NT> xcoords;
		for (auto v = e.vertices_begin(); v != e.vertices_end(); ++v) {
			if (e.is_standard(v)) {
				xcoords.insert(e.point(v).x());
			}
		}

		if (xcoords.size() < 2) { return std::vector<polygon_2>(); } // shouldn't ever happen

		std::vector<line_2> cut_lines;
		boost::transform(xcoords, std::back_inserter(cut_lines), [](const NT & x) { return line_2(point_2(x, 0), vector_2(0, 1)); });

		std::vector<nef_polygon_2> sections;
		for (size_t i = 0; i < cut_lines.size() - 1; ++i) {
			line_2 cut_a = cut_lines[i].opposite();
			line_2 cut_b = cut_lines[i + 1];
			esline_2 esline_a(cut_a.a(), cut_a.b(), cut_a.c());
			esline_2 esline_b(cut_b.a(), cut_b.b(), cut_b.c());
			nef_polygon_2 poly_a(esline_a, nef_polygon_2::EXCLUDED);
			nef_polygon_2 poly_b(esline_b, nef_polygon_2::EXCLUDED);
			sections.push_back(poly_a * poly_b);
		}

		boost::for_each(sections, [&res, this](const nef_polygon_2 & section) {
			nef_polygon_2 intr = section * *wrapped;
			nef_polygon_2::Explorer new_e = intr.explorer();
			for (auto f = new_e.faces_begin(); f != new_e.faces_end(); ++f) {
				if (f->mark()) {
					face new_face(new_e, f);
					boost::optional<polygon_2> poly = new_face.to_simple_polygon();
					if (poly) { res.push_back(*poly); }
				}
			}
		});
	}
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