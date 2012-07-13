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
	bool external;

	int calculate_level() const {
		if (external || layers.empty()) {
			return 2; 
		}
		else if (!spaces.second) {
			return layers.back().has_both_sides() ? 3 : 5;
		}
		else {
			return spaces.first == *spaces.second ? 4 : 2;
		}
	}

public:
	template <typename LayerRange>
	blockstack(
		area && a, 
		const LayerRange & layers, 
		bool base_sense, 
		const orientation * o, 
		bool is_external,
		const space * sp_a, 
		const NT & height_a,
		boost::optional<const space *> sp_b = boost::optional<const space *>(),
		boost::optional<NT> height_b = boost::optional<NT>())
		: a(a), o(o), base_sense(base_sense), layers(layers.begin(), layers.end()), spaces(sp_a, sp_b), heights(height_a, height_b), external(is_external)
	{ }

	const area & stack_area() const { return a; }

	template <typename OutputIterator>
	void to_surfaces(OutputIterator oi) const {
		oriented_area combined_geom(o, heights.first, a, base_sense); 
		std::vector<oriented_area> pieces;
		combined_geom.to_pieces(std::back_inserter(pieces));
		boost::for_each(pieces, [this, &oi](const oriented_area & piece) {
			if (!spaces.second) {
				std::shared_ptr<surface> surf(new surface(piece, layers.front().layer_element()));
				surf->set_space(spaces.first);
				surf->set_level(calculate_level());
				*oi++ = surf;
			}
			else if (layers.empty()) { // virtual
				std::shared_ptr<surface> surf1(new surface(piece, spaces.first));
				surf1->set_level(calculate_level());
				std::shared_ptr<surface> surf2(new surface(piece.reverse(), *spaces.second));
				surf2->set_level(calculate_level());
				surface::set_other_sides(surf1, surf2);
				*oi++ = surf1;
				*oi++ = surf2;
			}
			else {
				std::shared_ptr<surface> surf1(new surface(piece, layers.front().layer_element()));
				surf1->set_space(spaces.first);
				surf1->set_level(calculate_level());
				std::shared_ptr<surface> surf2(new surface(oriented_area(o, *heights.second, piece.area_2d(), !base_sense), layers.back().layer_element()));
				surf2->set_level(calculate_level());
				surf2->set_space(*spaces.second);
				surface::set_other_sides(surf1, surf2);
				*oi++ = surf1;
				*oi++ = surf2;
			}
		});
	}
};