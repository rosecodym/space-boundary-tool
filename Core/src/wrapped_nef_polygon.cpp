#include "precompiled.h"

#include "equality_context.h"
#include "geometry_common.h"
#include "nef_polygon_face.h"
#include "nef_polygon_vertex.h"

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

std::vector<vertex> wrapped_nef_polygon::get_vertices() const {
	current_explorer = wrapped->explorer();
	std::vector<vertex> res;
	for (auto v = current_explorer->vertices_begin(); v != current_explorer->vertices_end(); ++v) {
		if (v->mark() && current_explorer->is_standard(v)) {
			res.push_back(vertex(v));
		}
	}
	return res;
}

bool wrapped_nef_polygon::any_points_satisfy_predicate(const std::function<bool(point_2)> & pred) const {
	auto vertices = get_vertices();
	return boost::find_if(vertices, [&pred](const vertex & v) { 
		return pred(v.standard_point());
	}) != vertices.end();
}

bbox_2 wrapped_nef_polygon::bbox() const {
	auto vertices = get_vertices();
	bbox_2 res = vertices.front().bbox();
	boost::for_each(vertices, [&res](const vertex & v) { res = res + v.bbox(); });
	return res;
}

bool wrapped_nef_polygon::is_valid(double eps) const {
	if (is_empty()) { return true; }
	else if (*wrapped != wrapped->interior()) { return false; }
	auto points = get_vertices() | boost::adaptors::transformed([](const vertex & v) {
		return v.standard_point();
	});
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

} // namespace 

} // namespace