#pragma once

#include "precompiled.h"

#include "polygon_with_holes_2.h"

namespace geometry_2d {

namespace nef_polygons {

class face {
private:
	const nef_polygon_2::Explorer * e;
	nef_polygon_2::Explorer::Face_const_iterator f;
public:
	face(const nef_polygon_2::Explorer & explorer, nef_polygon_2::Explorer::Face_const_iterator iter)
		: e(&explorer), f(iter)
	{ }

	void print_with(const std::function<void(char *)> & func) const;
	boost::optional<polygon_2> to_simple_polygon() const;
	boost::optional<polygon_with_holes_2> to_pwh() const;
};

} // namespace nef_polygons

} // namespace geometry_2d