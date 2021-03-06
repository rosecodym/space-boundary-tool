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

bool wrapped_nef_polygon::is_valid(const equality_context & c) const {
	using geometry_common::smallest_squared_distance;
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
	auto dist = smallest_squared_distance(points.begin(), points.end());
	return !c.is_zero_squared(dist);
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
	boost::for_each(get_faces(), [&res](const face & f) {
		auto pwh = f.to_pwh();
		if (pwh) { res.push_back(*pwh); }
	});
	return res;
}

NT wrapped_nef_polygon::outer_regular_area() const {
	NT res = 0.0;
	boost::for_each(get_faces(), [&res](const face & f) {
		res += f.outer_regular_area();
	});
	return res;
}

wrapped_nef_polygon wrapped_nef_polygon::update_all(
	const std::function<point_2(point_2)> & update) const
{
	auto update_poly = [&update](const polygon_2 & poly) -> polygon_2 {
		polygon_2 res;
		for (auto p = poly.vertices_begin(); p != poly.vertices_end(); ++p) {
			res.push_back(update(*p));
		}
		return res;
	};
	std::vector<polygon_2> loops;
	boost::for_each(get_faces(), [&loops, &update_poly](const face & f) {
		auto pwh = f.to_pwh();
		if (pwh) {
			boost::transform(
				pwh->all_polygons(), 
				std::back_inserter(loops), 
				update_poly);
		}
	});
	return wrapped_nef_polygon(loops);
}

void wrapped_nef_polygon::clear() {
	wrapped->clear();
}

wrapped_nef_polygon & wrapped_nef_polygon::operator += (const wrapped_nef_polygon & other) {
	*wrapped = util::clean(*wrapped += other.wrapped->interior());
	return *this;
}

wrapped_nef_polygon & wrapped_nef_polygon::operator *= (const wrapped_nef_polygon & other) {
	*wrapped = util::clean(*wrapped *= other.wrapped->interior());
	return *this;
}

wrapped_nef_polygon & wrapped_nef_polygon::operator -= (const wrapped_nef_polygon & other) {
	*wrapped = util::clean(*wrapped -= other.wrapped->interior());
	return *this;
}

wrapped_nef_polygon & wrapped_nef_polygon::operator ^= (const wrapped_nef_polygon & other) {
	*wrapped = util::clean(*wrapped ^= other.wrapped->interior());
	return *this;
}

bool wrapped_nef_polygon::do_intersect(const wrapped_nef_polygon & lhs, const wrapped_nef_polygon & rhs) {
	return !(lhs * rhs).is_empty();
}

} // namespace 

} // namespace