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

	typedef std::vector<std::unique_ptr<surface>> block_group;
	std::vector<block_group> opening_surfaces;
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

		opening_surfaces.push_back(block_group());

		for (auto s = surfaces->begin(); s != surfaces->end(); ++s) {
			auto intr = (*s)->geometry() * gm_a;
			if (intr && !intr->area_2d().is_empty()) {
				auto surf = std::unique_ptr<surface>(new surface(
					*intr,
					e,
					(*s)->bounded_space(),
					layers,
					(*s)->is_external()));
				surf->set_parent_maybe(s->get(), height_eps);
				opening_surfaces.back().push_back(std::move(surf));
			}
			intr = (*s)->geometry() * gm_b;
			if (intr && !intr->area_2d().is_empty()) {
				auto surf = std::unique_ptr<surface>(new surface(
					*intr,
					e,
					(*s)->bounded_space(),
					layers,
					(*s)->is_external()));
				surf->set_parent_maybe(s->get(), height_eps);
				opening_surfaces.back().push_back(std::move(surf));
			}
		}
		reporting::report_progress(".");
	}
	for (auto g = opening_surfaces.begin(); g != opening_surfaces.end(); ++g) {
		for (auto p = g->begin(); p != g->end(); ++p) {
			for (auto q = p; q != g->end(); ++q) {
				if (p != q &&
					(*p)->geometry().sense() != (*q)->geometry().sense() &&
					(*p)->geometry().area_2d() == (*q)->geometry().area_2d())
				{
					surface::set_other_sides(*p, *q);
					break;
				}
			}
		}
		std::move(g->begin(), g->end(), std::back_inserter(*surfaces));
	}

	reporting::report_progress("done.\n");
}

}