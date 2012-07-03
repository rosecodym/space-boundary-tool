#include "precompiled.h"

#include <cstdarg>

#include "sbt-core.h"

#include "common.h"

namespace {

void set_vertex(polyloop * loop, size_t i, double x, double y, double z) {
	loop->vertices[i].x = x;
	loop->vertices[i].y = y;
	loop->vertices[i].z = z;
}

} // namespace

face create_face(size_t vertex_count, ...) {
	va_list ap;
	va_start(ap, vertex_count);
	simple_point v;

	face f;
	memset(&f, 0, sizeof(f));
	f.outer_boundary.vertex_count = vertex_count;
	f.outer_boundary.vertices = (point *)malloc(sizeof(point) * f.outer_boundary.vertex_count);
	for (size_t i = 0; i < vertex_count; ++i) {
		v = va_arg(ap, simple_point);
		set_vertex(&f.outer_boundary, i, v.x, v.y, v.z);
	}
	return f;
}

element_info * create_element_as_ext(
	const char * name, 
	element_type type, 
	material_id_t mat, 
	double dx,
	double dy,
	double dz,
	double depth,
	size_t v_count, 
	...)
{
	va_list ap;
	va_start(ap, v_count);

	element_info * res = (element_info *)malloc(sizeof(element_info));
	strcpy(res->id, name);
	res->material = mat;
	res->type = type;
	res->geometry.rep_type = REP_EXT;
	res->geometry.rep.as_ext.extrusion_depth = depth;
	res->geometry.rep.as_ext.ext_dx = dx;
	res->geometry.rep.as_ext.ext_dy = dy;
	res->geometry.rep.as_ext.ext_dz = dz;
	res->geometry.rep.as_ext.area = create_face(v_count, ap);
	return res;
}