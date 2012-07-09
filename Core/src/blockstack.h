#pragma once

#include "precompiled.h"

#include "area.h"
#include "layer_information.h"
#include "surface.h"

class space;
class orientation;

class blockstack {
private:
	area a;
	const orientation * o;
	bool base_sense;
	std::vector<layer_information> layers;
	std::pair<const space *, boost::optional<const space *>> spaces;
	std::pair<NT, boost::optional<NT>> heights;
public:
	template <typename LayerRange>
	blockstack(
		area && a, 
		const LayerRange & layers, 
		bool base_sense, 
		const orientation * o, 
		const space * sp_a, 
		const NT & height_a,
		boost::optional<const space *> sp_b = boost::optional<const space *>(),
		boost::optional<NT> height_b = boost::optional<NT>())
		: a(a), o(o), base_sense(base_sense), layers(layers.begin(), layers.end()), spaces(sp_a, sp_b), heights(height_a, height_b)
	{ }

	template <typename OutputIterator>
	void to_surfaces(OutputIterator oi) const {
		oriented_area geom_1(o, heights.first, a, base_sense); 
		if (!spaces.second) { // 3rd-level
			std::shared_ptr<surface> surf(new surface(std::move(geom_1), layers.front().layer_element()));
			surf->set_space(spaces.first);
			surf->set_level(3);
			*oi++ = surf;
		}
		else if (layers.empty()) { // virtual
			std::shared_ptr<surface> surf1(new surface(geom_1, spaces.first));
			surf1->set_level(2);
			std::shared_ptr<surface> surf2(new surface(geom_1.reverse(), *spaces.second));
			surf2->set_level(2);
			surface::set_other_sides(surf1, surf2);
			*oi++ = surf1;
			*oi++ = surf2;
		}
		else {
			std::shared_ptr<surface> surf1(new surface(geom_1, layers.front().layer_element()));
			surf1->set_level(2);
			std::shared_ptr<surface> surf2(new surface(oriented_area(o, *heights.second, a, !base_sense), layers.back().layer_element()));
			surf2->set_level(2);
			surface::set_other_sides(surf1, surf2);
			*oi++ = surf1;
			*oi++ = surf2;
		}
	}
};