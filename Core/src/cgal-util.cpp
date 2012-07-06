#include "precompiled.h"

#include "equality_context.h"
#include "geometry_common.h"
#include "misc-util.h"
#include "printing-macros.h"
#include "printing-util.h"
#include "sbt-core.h"
#include "wrapped_nef_polygon.h"

#include "cgal-util.h"

#define PRINT_GEOM(...) \
	do { \
		if (g_opts.flags & SBT_VERBOSE_GEOMETRY) { \
			sprintf(g_msgbuf, __VA_ARGS__); \
			NOTIFY_MSG( g_msgbuf); \
		} \
	} \
	while (false);

extern sb_calculation_options g_opts;
extern char g_msgbuf[256];

namespace util {

namespace cgal {

bool polygon_has_no_adjacent_duplicates(const polygon_2 & p, equality_context * c) {
	for (auto edge = p.edges_begin(); edge != p.edges_end(); ++edge) {
		if (c->are_effectively_same(edge->source(), edge->target())) {
			return false;
		}
	}
	return true;
}

bool is_axis_aligned(const polygon_2 & poly) {
	PRINT_2D_OPERATIONS("Entered util::cgal::is_axis_aligned.\n");
	for (auto edge = poly.edges_begin(); edge != poly.edges_end(); ++edge) {
		if (!(edge->is_vertical() || edge->is_horizontal())) {
			PRINT_2D_OPERATIONS("Exiting util::cgal::is_axis_aligned (result = false).\n");
			return false;
		}
	}
	PRINT_2D_OPERATIONS("Exiting util::cgal::is_axis_aligned (result = true).\n");
	return true;
}

bbox_3 nef_bounding_box(const nef_polyhedron_3 & nef) {
	nef_vertex_handle v;
	boost::optional<bbox_3> res;
	CGAL_forall_vertices(v, nef) {
		if (!res.is_initialized()) {
			res = v->point().bbox();
		}
		else {
			res = *res + v->point().bbox();
		}
	}
	return *res;
}

} // namespace cgal

} // namespace util