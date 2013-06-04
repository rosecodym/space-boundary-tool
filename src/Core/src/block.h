#pragma once

#include "precompiled.h"

#include "area.h"
#include "layer_information.h"
#include "orientation.h"
#include "oriented_area.h"
#include "surface.h"

class element;

class block {
private:
	const orientation * o;
	geometry_2d::area a;
	layer_information layer;
	bool base_sense;

public:
	block(const oriented_area & single_halfblock, const element & e)
		: o(&single_halfblock.orientation()), 
		a(single_halfblock.area_2d()),
		layer(single_halfblock.height(), e),
		base_sense(single_halfblock.sense())
	{ }

	block(const oriented_area & halfblock_a, const oriented_area & halfblock_b, const element & e)
		: o(&halfblock_a.orientation()),
		a(halfblock_a.area_2d()),
		layer(halfblock_a.height(), halfblock_b.height(), e),
		base_sense(halfblock_a.sense())
	{ }

	block(block && src)
		: o(src.o),
		a(std::move(src.a)),
		layer(std::move(src.layer)),
		base_sense(src.base_sense)
	{ }

	block & operator = (block && src) {
		if (&src != this) {
			o = src.o;
			a = std::move(src.a);
			layer = std::move(src.layer);
			base_sense = src.base_sense;
		}
		return *this;
	}

	std::pair<NT, boost::optional<NT>> heights() const { 
		return layer.has_both_sides() ? std::make_pair(layer.height_a(), boost::optional<NT>(layer.height_b())) : std::make_pair(layer.height_a(), boost::optional<NT>());
	}
	bool sense() const { return base_sense; }
	const geometry_2d::area & base_area() const { return a; }
	const orientation * block_orientation() const { return o; }
	const layer_information & material_layer() const { return layer; }
	bool is_fenestration() const { return layer.layer_element().is_fenestration(); }
};