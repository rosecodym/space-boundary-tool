#pragma once

#include "precompiled.h"

#include "sbt-core.h"

struct simple_point {
	double x;
	double y;
	double z;
	simple_point() { }
	simple_point(double x, double y, double z) : x(x), y(y), z(z) { }
};

face create_face(size_t vertex_count, ...);

element_info * create_element_as_ext(
	const char * name, 
	element_type type, 
	material_id_t mat, 
	double dx,
	double dy,
	double dz,
	double depth,
	size_t v_count, 
	...);