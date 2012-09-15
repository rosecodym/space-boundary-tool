#include "precompiled.h"

#include "cleanup_loop.h"
#include "nef_polygon_util.h"
#include "sbt-core.h"

#include "nef_polygon_face.h"

extern sb_calculation_options g_opts;

namespace geometry_2d {

namespace nef_polygons {

void face::print_with(const std::function<void(char *)> & func) const {
	func("Face:\n");
	auto p = e->face_cycle(f);
	auto end = p;
	CGAL_For_all(p, end) {
		if (e->is_standard(p->vertex())) {
			char buf[256];
			sprintf(buf, "(%f, %f)\n", CGAL::to_double(p->vertex()->point().x()), CGAL::to_double(p->vertex()->point().y()));
			func(buf);
		}
		else {
			func("(non-standard point)\n");
		}
	}
	for (auto h = e->holes_begin(f); h != e->holes_end(f); ++h) {
		func("Hole:\n");
		nef_polygon_2::Explorer::Halfedge_around_face_const_circulator curr = h;
		auto end = curr;
		CGAL_For_all(curr, end) {
			if (e->is_standard(curr->vertex())) {
				char buf[256];
				sprintf(buf, "(%f, %f)\n", CGAL::to_double(curr->vertex()->point().x()), CGAL::to_double(curr->vertex()->point().y()));
				func(buf);
			}
			else {
				func("(non-standard point)\n");
			}
		}
	}
}

boost::optional<polygon_with_holes_2> face::to_pwh() const {
	polygon_2 outer;
	std::vector<polygon_2> holes;
	auto p = e->face_cycle(f);
	auto end = p;
	CGAL_For_all(p, end) {
		if (e->is_standard(p->vertex())) {
			outer.push_back(util::to_point(p->vertex()->point()));
		}
	}
	if (!geometry_common::cleanup_loop(&outer, g_opts.equality_tolerance)) {
		return boost::optional<polygon_with_holes_2>();
	}
	for (auto h = e->holes_begin(f); h != e->holes_end(f); ++h) {
		nef_polygon_2::Explorer::Halfedge_around_face_const_circulator curr = h;
		auto end = curr;
		holes.push_back(polygon_2());
		CGAL_For_all(curr, end) {
			if (e->is_standard(curr->vertex())) {
				holes.back().push_back(util::to_point(curr->vertex()->point()));
			}
		}
		if (!geometry_common::cleanup_loop(&holes.back(), g_opts.equality_tolerance)) {
			holes.pop_back();
		}
	}
	return polygon_with_holes_2(outer, holes);
}

} // namespace nef_polygons

} // namespace geometry_2d