#pragma once

#include "precompiled.h"

#include "block.h"
#include "layer_information.h"
#include "report.h"
#include "surface.h"

namespace opening_assignment {

template <typename SurfaceRange, typename OpeningBlocks>
void assign_openings(
	SurfaceRange * surfaces, 
	const OpeningBlocks & openings,
	double height_eps)
{
	if (std::distance(openings.begin(), openings.end()) == 0) {
		reporting::report_progress("No openings to assign.\n");
		return;
	}
	std::vector<std::unique_ptr<surface>> opening_surfaces;
	reporting::report_progress("Assigning openings");
	for (auto blk = openings.begin(); blk != openings.end(); ++blk) {
		assert(blk->heights().second);
		auto o = blk->block_orientation();
		auto blk_area = blk->base_area();
		NT height_a = blk->heights().first;
		NT height_b = *blk->heights().second;
		// The senses need to be flipped because we're coming straight from
		// blocks.
		oriented_area gm_a(o, height_a, blk_area, !blk->sense());
		oriented_area gm_b(o, height_b, blk_area, blk->sense());
		std::vector<layer_information> layers;
		const element & e = blk->material_layer().layer_element();
		layers.push_back(layer_information(height_a, height_b, e));
		std::unique_ptr<surface> surf_a;
		std::unique_ptr<surface> surf_b;
		for (auto s = surfaces->begin(); s != surfaces->end(); ++s) {
			if (!surf_a && (*s)->geometry().contains(gm_a, height_eps)) {
				surf_a = std::unique_ptr<surface>(new surface(
					std::move(gm_a),
					e,
					(*s)->bounded_space(),
					layers,
					(*s)->is_external()));
				surf_a->set_parent_maybe(s->get(), height_eps);
				if (surf_a->is_external()) {
					break;
				}
			}
			else if (!surf_b && (*s)->geometry().contains(gm_b, height_eps)) {
				surf_b = std::unique_ptr<surface>(new surface(
					std::move(gm_b),
					e,
					(*s)->bounded_space(),
					layers,
					(*s)->is_external()));
				surf_b->set_parent_maybe(s->get(), height_eps);
				if (surf_b->is_external()) {
					break;
				}
			}
		}
		if (surf_a && surf_b) { surface::set_other_sides(surf_a, surf_b); }
		if (surf_a || surf_b) { reporting::report_progress("."); }
		if (surf_a) { opening_surfaces.push_back(std::move(surf_a)); }
		if (surf_b) { opening_surfaces.push_back(std::move(surf_b)); }
	}
	std::move(
		opening_surfaces.begin(), 
		opening_surfaces.end(), 
		std::back_inserter(*surfaces));
	reporting::report_progress("done.\n");
}