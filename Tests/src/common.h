#pragma once

#include "precompiled.h"

#include <gtest/gtest.h>

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

solid create_brep(size_t face_count, ...);
solid create_ext(double dx, double dy, double dz, double depth, face base);

element_info * create_element(
	const char * name, 
	element_type type, 
	element_id_t id, 
	solid geometry);
space_info * create_space(const char * name, solid geometry);

inline element_info * create_dummy_element() {
	return create_element("dummy element", WALL, 1,
		create_ext(0, 0, 1, 1, create_face(4,
			simple_point(0, 0, 0),
			simple_point(0, 1, 0),
			simple_point(1, 1, 0),
			simple_point(1, 0, 0))));
}

inline space_info * create_dummy_space() {
	return create_space("dummy space", create_ext(0, 0, 1, 1, create_face(4,
		simple_point(0, 0, 0),
		simple_point(0, 1, 0),
		simple_point(1, 1, 0),
		simple_point(1, 0, 0))));
}