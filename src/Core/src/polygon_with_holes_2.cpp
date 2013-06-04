#include "precompiled.h"

#include "cleanup_loop.h"
#include "geometry_common.h"
#include "report.h"
#include "sbt-core.h"

#include "polygon_with_holes_2.h"

extern sb_calculation_options g_opts;

void polygon_with_holes_2::cleanup() {
	geometry_common::cleanup_loop(&m_outer, EPS_MAGIC);
	boost::for_each(m_holes, [](polygon_2 & hole) {
		geometry_common::cleanup_loop(&hole, EPS_MAGIC);
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