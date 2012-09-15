#pragma once

#include "precompiled.h"

#include "nef_polygon_util.h"

namespace geometry_2d {

namespace nef_polygons {

class vertex {
private:
	nef_polygon_2::Explorer::Vertex_const_iterator v;
public:
	vertex(nef_polygon_2::Explorer::Vertex_const_iterator v) : v(v)
	{ }

	point_2 standard_point() const { return util::to_point(v->point()); }

	bbox_2 bbox() const { return v->point().bbox(); }
};

} // namespace nef_polygons

} // namespace geometry_2d