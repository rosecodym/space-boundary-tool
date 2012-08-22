#pragma once

#include "precompiled.h"

#include "polygon_with_holes_3.h"
#include "sbt-core.h"

struct simple_point {
	double x;
	double y;
	double z;
	simple_point() { }
	simple_point(double x, double y, double z) : x(x), y(y), z(z) { }
};

face create_face(size_t vertex_count, ...);

solid create_ext(double dx, double dy, double dz, double depth, face base);

element_info * create_element(const char * name, element_type type, material_id_t mat, solid geometry);
space_info * create_space(const char * name, solid geometry);