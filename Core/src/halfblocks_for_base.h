#pragma once

#include "precompiled.h"

#include "geometry_common.h"
#include "surface_pair.h"

namespace blocking {

namespace impl {

template <typename OutputIterator>
void halfblocks_for_base(const relations_grid & surface_relationships, size_t base_index, equality_context * flattening_context, OutputIterator oi) {
	typedef relations_grid::index_range array_range;
	typedef relations_grid::index_gen indices;

	const auto & row = surface_relationships[indices()[base_index][array_range()]];

	if (boost::count_if(row, [](const surface_pair & pair) {
		return pair.contributes_to_envelope();
	}) == 1)
	{
		*oi++ = row[0].base();
		return;
	}
	
	typedef CGAL::Envelope_diagram_2<Surface_pair_envelope_traits> envelope_diagram;
	envelope_diagram diag;
	CGAL::lower_envelope_3(row.begin(), row.end(), diag);

	const oriented_area & base = surface_relationships[base_index][base_index].base();

	for (auto p = diag.faces_begin(); p != diag.faces_end(); ++p) {
		if (!p->is_unbounded()) {
			polygon_2 this_poly;
			auto ccb = p->outer_ccb();
			auto end = ccb;
			CGAL_For_all(ccb, end) {
				this_poly.push_back(flattening_context->snap(ccb->target()->point()));
			}
			// for some reason the envelope calculation creates degenerate faces sometimes
			if (!geometry_common::cleanup_loop(&this_poly, g_opts.equality_tolerance)) {
				continue;
			}
			// we have to do two passes because of a bug in geometry_common::cleanup_loop. see issue #4
			// update: should be resolved but i haven't gotten around to testing this particular path with this removed
			if (!geometry_common::cleanup_loop(&this_poly, g_opts.equality_tolerance)) {
				continue;
			}
			area this_area(this_poly);
			area intr = base.area_2d() * this_area;
			if (!intr.is_empty()) {
				*oi++ = oriented_area(base, std::move(intr));
			}
		}
	}
}

} // namespace impl

} // namespace blocking