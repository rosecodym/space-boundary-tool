#pragma once

#include "precompiled.h"

#include "block.h"
#include "layer_information.h"
#include "report.h"
#include "surface.h"

namespace opening_assignment {

namespace impl {

boost::optional<oriented_area> has_subarea(
	const oriented_area & parent,
	const oriented_area & child,
	double height_eps);

template <typename SurfaceRange>
std::vector<std::unique_ptr<surface>> place_opening_block(
	const block & opening,
	const SurfaceRange & surfaces,
	double height_eps)
{
	std::vector<std::unique_ptr<surface>> res;
	if (!opening.heights().second) { return res; }

	auto o = opening.block_orientation();
	auto blk_area = opening.base_area();
	NT height_a = opening.heights().first;
	NT height_b = *opening.heights().second;
	// The senses need to be flipped because we're coming straight from blocks.
	oriented_area gm_a(o, height_a, blk_area, !opening.sense());
	oriented_area gm_b(o, height_b, blk_area, opening.sense());

	std::vector<layer_information> layers;
	const element & e = opening.material_layer().layer_element();
	layers.push_back(layer_information(height_a, height_b, e));

	for (auto s = surfaces.begin(); s != surfaces.end(); ++s) {
		const oriented_area & sgeom = (*s)->geometry();
		auto subarea = has_subarea(sgeom, gm_a, height_eps);
		if (subarea) {
			auto surf = std::unique_ptr<surface>(new surface(
				*subarea,
				e,
				(*s)->bounded_space(),
				layers,
				(*s)->is_external()));
			surf->set_parent_maybe(s->get(), height_eps);
			res.push_back(std::move(surf));
		}
		subarea = has_subarea(sgeom, gm_b, height_eps);
		if (subarea) {
			auto surf = std::unique_ptr<surface>(new surface(
				*subarea,
				e,
				(*s)->bounded_space(),
				layers,
				(*s)->is_external()));
			surf->set_parent_maybe(s->get(), height_eps);
			res.push_back(std::move(surf));
		}
	}
	return res;
}

} // namespace impl

template <typename SurfaceRange, typename OpeningBlocks>
void assign_openings(
	SurfaceRange * surfaces, 
	const OpeningBlocks & openings,
	double height_eps)
{
	typedef std::vector<std::unique_ptr<surface>> block_group;

	if (std::distance(openings.begin(), openings.end()) == 0) {
		reporting::report_progress("No openings to assign.\n");
		return;
	}

	std::vector<block_group> opening_surfaces;
	reporting::report_progress("Assigning openings");

	for (auto blk = openings.begin(); blk != openings.end(); ++blk) {
		auto placed = impl::place_opening_block(*blk, *surfaces, height_eps);
		opening_surfaces.push_back(std::move(placed));
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
					assert(!(*p)->parent()->is_external());
					assert(!(*q)->parent()->is_external());
					break;
				}
			}
		}
		std::move(g->begin(), g->end(), std::back_inserter(*surfaces));
	}

	reporting::report_progress("done.\n");
}

}