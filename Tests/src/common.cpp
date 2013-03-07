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
	va_end(ap);
	return f;
}

solid create_brep(size_t face_count, ...) {
	va_list ap;
	va_start(ap, face_count);
	
	solid s;
	memset(&s, 0, sizeof(solid));
	s.rep_type = REP_BREP;
	brep & b = s.rep.as_brep;
	b.face_count = face_count;
	b.faces = (face *)malloc(sizeof(face) * b.face_count);
	for (size_t i = 0; i < face_count; ++i) {
		b.faces[i] = va_arg(ap, face);
	}
	va_end(ap);
	return s;
}

solid create_ext(double dx, double dy, double dz, double depth, face geometry) {
	solid res;
	res.rep_type = REP_EXT;
	res.rep.as_ext.extrusion_depth = depth;
	res.rep.as_ext.ext_dx = dx;
	res.rep.as_ext.ext_dy = dy;
	res.rep.as_ext.ext_dz = dz;
	res.rep.as_ext.area = geometry;
	return res;
}

element_info * create_element(
	const char * name, 
	element_type type, 
	element_id_t mat, 
	solid geometry) 
{
	element_info * res = (element_info *)malloc(sizeof(element_info));
	strcpy(res->name, name);
	res->id = mat;
	res->type = type;
	res->geometry = geometry;
	return res;
}

space_info * create_space(const char * name, solid geometry) {
	space_info * res = (space_info *)malloc(sizeof(space_info));
	strcpy(res->id, name);
	res->geometry = geometry;
	return res;
}