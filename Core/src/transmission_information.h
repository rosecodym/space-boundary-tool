#pragma once

#include "precompiled.h"

#include "area.h"
#include "layer_information.h"
#include "surface.h"

class orientation;
class space;

class transmission_information {
private:
	area a_;
	const orientation * o_;
	bool base_sense_;
	std::vector<layer_information> layers_;
	const space * start_space_;
	const space * end_space_;
	NT start_height_;
	boost::optional<NT> end_height_;
	bool external_;

public:
	template <typename LayerRange>
	transmission_information(
		const area & a,
		const LayerRange & layers,
		bool base_sense,
		const orientation * o,
		bool external,
		const space * start_space,
		const NT & start_height,
		const space * end_space = nullptr,
		const boost::optional<NT> & end_height = boost::optional<NT>())
		: a_(a),
		  o_(o),
		  base_sense_(base_sense),
		  layers_(layers.begin(), layers.end()),
		  start_space_(start_space),
		  end_space_(end_space),
		  start_height_(start_height),
		  end_height_(end_height),
		  external_(external)
	{ 
		assert(layers_.size() > 0 || end_space != nullptr);
	}

	const area & common_area() const { return a_; }

	template <typename SurfaceOutputIterator>
	void to_surfaces(SurfaceOutputIterator oi) const {
		typedef std::unique_ptr<surface> surf_ptr;

		oriented_area start_geom(o_, start_height_, a_, base_sense_);
		std::vector<oriented_area> pieces;
		start_geom.to_pieces(std::back_inserter(pieces));
		for (auto p = pieces.begin(); p != pieces.end(); ++p) {
			if (end_space_ == nullptr) {
				if (layers_.back().has_both_sides()) {
					// external or non-transmitting due to thickness
					*oi++ = surf_ptr(new surface(
						*p, 
						layers_.front().layer_element(),
						*start_space_,
						layers_,
						external_));
				}
				else {
					// non-transmitting due to halfblock
					*oi++ = surf_ptr(new surface(
						*p,
						layers_.front().layer_element(),
						*start_space_,
						std::vector<layer_information>(),
						false));
				}
			}
			else if (layers_.empty()) {
				// virtual
				surf_ptr surf1(new surface(*p, *start_space_));
				surf_ptr surf2(new surface(p->reverse(), *end_space_));
				surface::set_other_sides(surf1, surf2);
				*oi++ = std::move(surf1);
				*oi++ = std::move(surf2);
			}
			else {
				// transmitting internal
				surf_ptr surf1(new surface(
					*p,
					layers_.front().layer_element(),
					*start_space_,
					layers_,
					false));
				surf_ptr surf2(new surface(
					oriented_area(o_, *end_height_, p->area_2d(), !base_sense_),
					layers_.back().layer_element(),
					*end_space_,
					layers_ | boost::adaptors::reversed,
					false));
				surface::set_other_sides(surf1, surf2);
				*oi++ = std::move(surf1);
				*oi++ = std::move(surf2);
			}
		}
	}
};