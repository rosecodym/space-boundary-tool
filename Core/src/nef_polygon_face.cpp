#include "precompiled.h"

#include "cleanup_loop.h"
#include "nef_polygon_util.h"
#include "sbt-core.h"

#include "nef_polygon_face.h"

extern sb_calculation_options g_opts;

namespace geometry_2d {

namespace nef_polygons {

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
	polygon_2 res;
	auto p = e->face_cycle(f);
	auto end = p;
	CGAL_For_all(p, end) {
		if (!e->is_standard(p->vertex())) { return boost::optional<polygon_2>(); }
		auto pt = e->point(p->vertex());
		res.push_back(point_2(pt.x(), pt.y()));
	}
	return res;
}

boost::optional<polygon_with_holes_2> face::to_pwh() const {
	polygon_2 outer;
	std::vector<polygon_2> holes;
	auto p = e->face_cycle(f);
	auto end = p;
	CGAL_For_all(p, end) {
		if (e->is_standard(p->vertex())) {
			auto pt = e->point(p->vertex());
			outer.push_back(point_2(pt.x(), pt.y()));
		}
	}
	if (!geometry_common::cleanup_loop(&outer, EPS_MAGIC)) {
		return boost::optional<polygon_with_holes_2>();
	}
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
		if (!geometry_common::cleanup_loop(&holes.back(), EPS_MAGIC)) {
			holes.pop_back();
		}
	}
	return polygon_with_holes_2(outer, holes);
}

} // namespace nef_polygons

} // namespace geometry_2d