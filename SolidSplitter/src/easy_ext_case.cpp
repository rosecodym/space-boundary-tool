#include "precompiled.h"

#include "../../Core/src/sbt-core.h"

#include "cgal-typedefs.h"

#include "easy_ext_case.h"

void handle_easy_ext_case(
	const extruded_area_solid & ext, 
	const vector_3 & vec, 
	double thicknesses[], 
	int layer_count, 
	solid results[])
{
	NT current_depth = 0;
	for (int i = 0; i < layer_count; ++i) {
		solid & s = results[i];
		s.rep_type = REP_EXT;
		extruded_area_solid & e = s.rep.as_ext;
		e.extrusion_depth = thicknesses[i];
		e.ext_dx = CGAL::to_double(vec.x());
		e.ext_dy = CGAL::to_double(vec.y());
		e.ext_dz = CGAL::to_double(vec.z());
		vector_3 delta(vec * current_depth);
		e.area.outer_boundary.vertex_count = ext.area.outer_boundary.vertex_count;
		e.area.outer_boundary.vertices = (point *)malloc(sizeof(point) * e.area.outer_boundary.vertex_count);
		for (size_t j = 0; j < ext.area.outer_boundary.vertex_count; ++j) {
			point_3 newpoint = point_3(ext.area.outer_boundary.vertices[j].x, ext.area.outer_boundary.vertices[j].y, ext.area.outer_boundary.vertices[j].z) + delta;
			e.area.outer_boundary.vertices[j].x = to_double(newpoint.x());
			e.area.outer_boundary.vertices[j].y = to_double(newpoint.y());
			e.area.outer_boundary.vertices[j].z = to_double(newpoint.z());
		}
		e.area.void_count = ext.area.void_count;
		if (e.area.void_count > 0) {
			e.area.void_count = ext.area.void_count;
			e.area.voids = (polyloop *)malloc(sizeof(polyloop) * e.area.void_count);
			for (size_t j = 0; j < ext.area.void_count; ++j) {
				e.area.voids[j].vertex_count = ext.area.voids[j].vertex_count;
				e.area.voids[j].vertices = (point *)malloc(sizeof(point) * e.area.voids[j].vertex_count);
				for (size_t k = 0; k < ext.area.voids[j].vertex_count; ++k) {
					point_3 newpoint = point_3(ext.area.voids[j].vertices[k].x, ext.area.voids[j].vertices[k].y, ext.area.voids[j].vertices[k].z) + delta;
					e.area.voids[j].vertices[k].x = to_double(newpoint.x());
					e.area.voids[j].vertices[k].y = to_double(newpoint.y());
					e.area.voids[j].vertices[k].z = to_double(newpoint.z());
				}
			}
		}
		else {
			e.area.voids = nullptr;
		}
		current_depth += thicknesses[i];
	}
}