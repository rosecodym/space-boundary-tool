#include "precompiled.h"

#include "cleanup_loop.h"
#include "geometry_common.h"
#include "printing-macros.h"
#include "sbt-core.h"

#include "polygon_with_holes_2.h"

extern sb_calculation_options g_opts;

void polygon_with_holes_2::cleanup() {
	bool cleanup_ok = geometry_common::cleanup_loop(&m_outer, g_opts.equality_tolerance);
	if (!cleanup_ok && FLAGGED(SBT_EXPENSIVE_CHECKS)) {
		ERROR_MSG("Couldn't clean up a pwh_2's outer boundary:\n");
		PRINT_POLYGON(m_outer);
		abort();
	}
	boost::for_each(m_holes, [](polygon_2 & hole) {
		bool cleanup_ok = geometry_common::cleanup_loop(&hole, g_opts.equality_tolerance);
		if (!cleanup_ok && FLAGGED(SBT_EXPENSIVE_CHECKS)) {
			ERROR_MSG("Couldn't clean up a pwh_2 hole:\n");
			PRINT_POLYGON(hole);
			abort();
		}
	});
}

void polygon_with_holes_2::reverse() {
	m_outer.reverse_orientation();
	boost::for_each(m_holes, [](polygon_2 & poly) {
		poly.reverse_orientation();
	});
}

bool polygon_with_holes_2::is_axis_aligned() const {
	return 
		geometry_common::is_axis_aligned(m_outer) &&
		boost::find_if(m_holes, [](const polygon_2 & hole) { return !geometry_common::is_axis_aligned(hole); }) == m_holes.end();
}

std::vector<polygon_2> polygon_with_holes_2::all_polygons() const { 
	std::vector<polygon_2> res(m_holes.begin(), m_holes.end());
	res.push_back(m_outer);
	return res;
}