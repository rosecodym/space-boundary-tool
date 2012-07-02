#pragma once

#include "precompiled.h"

#include "cgal-util.h"
#include "geometry_2d_common.h"
#include "printing-macros.h"
#include "surface_pair.h"

namespace blocking {

namespace impl {

template <typename OutputIterator>
void halfblocks_for_base(const relations_grid & surface_relationships, size_t base_index, equality_context * flattening_context, OutputIterator oi) {
	typedef relations_grid::index_range array_range;
	typedef relations_grid::index_gen indices;

	const auto & row = surface_relationships[indices()[base_index][array_range()]];
	
	PRINT_BLOCKS("Calculating halfblocks for a base (%u row entries).\n", row.size());

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
	PRINT_BLOCKS("Envelope calculation completed. %u faces resulted.\n", diag.number_of_faces());

	const oriented_area & base = surface_relationships[base_index][base_index].base();

	for (auto p = diag.faces_begin(); p != diag.faces_end(); ++p) {
		if (!p->is_unbounded()) {
			PRINT_BLOCKS("Processing diagram face.\n");
			polygon_2 this_poly;
			auto ccb = p->outer_ccb();
			auto end = ccb;
			CGAL_For_all(ccb, end) {
				this_poly.push_back(flattening_context->snap(ccb->target()->point()));
			}
			// for some reason the envelope calculation creates degenerate faces sometimes
			if (!geometry_2d::cleanup_polygon(&this_poly, g_opts.equality_tolerance)) {
				continue;
			}
			area this_area(this_poly);
			if (FLAGGED(SBT_VERBOSE_BLOCKS)) {
				NOTIFY_MSG("Intersecting base area:\n");
				base.area_2d().print();
				NOTIFY_MSG("With face area:\n");
				this_area.print();
			}
			area intr = base.area_2d() * this_area;
			PRINT_BLOCKS("Intersection calculated.\n");
			if (!intr.is_empty()) {
				*oi++ = oriented_area(base, std::move(intr));
				PRINT_BLOCKS("Created halfblock.\n");
			}
		}
	}
}

} // namespace impl

} // namespace blocking