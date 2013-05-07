#include "precompiled.h"

#include "cleanup_loop.h"
#include "report.h"
#include "sbt-core.h"

#include "nef_polygon_util.h"

extern sb_calculation_options g_opts;

namespace geometry_2d {

namespace nef_polygons {

namespace util {

nef_polygon_2 clean(const nef_polygon_2 & nef) {
	typedef std::vector<espoint_2> loop_t;

	if (nef.is_empty()) { return nef; }

	nef_polygon_2::Explorer e = nef.explorer();
	std::vector<loop_t> loops;

	for (auto f = e.faces_begin(); f != e.faces_end(); ++f) {
		if (f->mark()) {
			loops.push_back(loop_t());
			auto p = e.face_cycle(f);
			auto end = p;
			CGAL_For_all(p, end) {
				if (e.is_standard(p->vertex())) {
					loops.back().push_back(eK().standard_point(p->vertex()->point()));
				}
			}
			if (loops.back().empty()) {
				loops.pop_back();
				continue;
			}
			for (auto h = e.holes_begin(f); h != e.holes_end(f); ++h) {
				loops.push_back(loop_t());
				nef_polygon_2::Explorer::Halfedge_around_face_const_circulator p = h;
				auto end = p;
				if (p != end) {
					CGAL_For_all(p, end) {
						if (e.is_standard(p->vertex())) {
							loops.back().push_back(eK().standard_point(p->vertex()->point()));
						}
					}
				}
			}
		}
	}

	bool changed = false;
	boost::for_each(loops, [&changed](loop_t & loop) { 
		size_t in_size = loop.size();
		if (!geometry_common::cleanup_loop(&loop, EPS_MAGIC)) { loop.clear(); }
		if (loop.size() != in_size) { changed = true; }
	});

	if (!changed) { return nef; }
	else {
		nef_polygon_2 res(nef_polygon_2::EMPTY);
		boost::for_each(loops, [&res](const loop_t & loop) {
			res ^= nef_polygon_2(loop.begin(), loop.end(), nef_polygon_2::EXCLUDED);
		});
		res = res.interior();
		return res;
	}
}

nef_polygon_2 create_nef_polygon(polygon_2 poly) {
	if (!geometry_common::cleanup_loop(&poly, EPS_MAGIC)) {
		return nef_polygon_2::EMPTY;
	}
	std::vector<espoint_2> ext;
	std::transform(
		poly.vertices_begin(), 
		poly.vertices_end(), 
		std::back_inserter(ext), [](const point_2 & p) { 
			return to_espoint(p); 
		});
	return poly.is_counterclockwise_oriented() ?
		nef_polygon_2(ext.begin(), ext.end(), nef_polygon_2::EXCLUDED) :
		nef_polygon_2(ext.rbegin(), ext.rend(), nef_polygon_2::EXCLUDED);
}

espoint_2 to_espoint(const point_2 & p) { 
	return espoint_2(p.x(), p.y()); 
}

size_t vertex_count(const nef_polygon_2 & nef) {
	// subtract the corners of the infimaximal box
	return nef.explorer().number_of_vertices() - 4; 
}

} // namespace util

} // namespace nef_polygons

} // namespace geometry_2d