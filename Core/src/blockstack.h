#pragma once

#include "precompiled.h"

#include "area.h"
#include "layer_information.h"
#include "oriented_area.h"
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
				std::unique_ptr<surface> surf(new surface(piece, layers.front().layer_element(), *spaces.first, layers, external));
				*oi++ = std::move(surf);
			}
			else if (layers.empty()) { // virtual
				std::unique_ptr<surface> surf1(new surface(piece, *spaces.first));
				std::unique_ptr<surface> surf2(new surface(piece.reverse(), **spaces.second));
				surface::set_other_sides(surf1, surf2);
				*oi++ = std::move(surf1);
				*oi++ = std::move(surf2);
			}
			else {
				std::unique_ptr<surface> surf1(new surface(
					piece, 
					layers.front().layer_element(), 
					*spaces.first, 
					layers, 
					false));
				std::unique_ptr<surface> surf2(new surface(
					oriented_area(o, *heights.second, piece.area_2d(), !base_sense), 
					layers.back().layer_element(), 
					**spaces.second, 
					layers | boost::adaptors::reversed, 
					false));
				surface::set_other_sides(surf1, surf2);
				*oi++ = std::move(surf1);
				*oi++ = std::move(surf2);
			}
		});
	}
};