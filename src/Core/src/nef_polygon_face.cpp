#include "precompiled.h"

#include "nef_polygon_face.h"

#include "cleanup_loop.h"
#include "geometry_common.h"
#include "nef_polygon_util.h"
#include "sbt-core.h"

extern sb_calculation_options g_opts;

namespace geometry_2d {

namespace nef_polygons {

boost::optional<polygon_2> face::outer() const {
	polygon_2 res;
	auto p = e->face_cycle(f);
	auto end = p;
	CGAL_For_all(p, end) {
		if (e->is_standard(p->vertex())) {
			auto pt = e->point(p->vertex());
			res.push_back(point_2(pt.x(), pt.y()));
		}
	}
	if (!geometry_common::cleanup_loop(&res, g_opts.tolernace_in_meters)) {
		return boost::optional<polygon_2>();
	}
	else {
		return res;
	}
}

std::string face::to_string() const {
	std::stringstream ss;
	ss << "Face:\n";
	auto p = e->face_cycle(f);
	auto end = p;
	CGAL_For_all(p, end) {
		if (e->is_standard(p->vertex())) {
			ss << (boost::format("(%f, %f)\n") % CGAL::to_double(p->vertex()->point().x()) % CGAL::to_double(p->vertex()->point().y())).str();
		}
		else {
			ss << "(non-standard point)\n";
		}
	}
	for (auto h = e->holes_begin(f); h != e->holes_end(f); ++h) {
		ss << "Hole:\n";
		nef_polygon_2::Explorer::Halfedge_around_face_const_circulator curr = h;
		auto end = curr;
		CGAL_For_all(curr, end) {
			if (e->is_standard(curr->vertex())) {
				ss << (boost::format("(%f, %f)\n") % CGAL::to_double(curr->vertex()->point().x()) % CGAL::to_double(curr->vertex()->point().y())).str();
			}
			else {
				ss << "(non-standard point)\n";
			}
		}
	}
	return ss.str();
}

boost::optional<polygon_2> face::to_simple_polygon() const {
	if (e->holes_begin(f) != e->holes_end(f)) { return boost::optional<polygon_2>(); }
	else { return outer(); }
}

boost::optional<polygon_with_holes_2> face::to_pwh() const {
	auto out = outer();
	if (!out) { return boost::optional<polygon_with_holes_2>(); }
	std::vector<polygon_2> holes;
	for (auto h = e->holes_begin(f); h != e->holes_end(f); ++h) {
		nef_polygon_2::Explorer::Halfedge_around_face_const_circulator curr = h;
		auto end = curr;
		holes.push_back(polygon_2());
		CGAL_For_all(curr, end) {
			if (e->is_standard(curr->vertex())) {
				auto pt = e->point(curr->vertex());
				holes.back().push_back(point_2(pt.x(), pt.y()));
			}
		}
		if (!geometry_common::cleanup_loop(&holes.back(), g_opts.tolernace_in_meters)) {
			holes.pop_back();
		}
	}
	return polygon_with_holes_2(*out, holes);
}

NT face::outer_regular_area() const {
	auto out = outer();
	return out ? geometry_common::regular_area(*out) : 0.0;
}

} // namespace nef_polygons

} // namespace geometry_2d