#pragma once

#include "precompiled.h"

#include "geometry_common.h"
#include "surface_pair.h"

namespace blocking {

namespace impl {

template <typename OutputIterator>
void halfblocks_for_base(
	const relations_grid & surf_rels, 
	size_t base_index, 
	equality_context * result_ctxt, 
	OutputIterator oi) 
{
	typedef relations_grid::index_range array_range;
	typedef relations_grid::index_gen indices;

	const auto & row = surf_rels[indices()[base_index][array_range()]];

	size_t contributing_pairs = boost::count_if(
		row,
		[](const surface_pair & pair) {
			return pair.contributes_to_envelope();
		});
	if (contributing_pairs == 0 || contributing_pairs == 1)
	{
		*oi++ = row[0].base();
		return;
	}

	equality_context flat_ctxt(result_ctxt->height_epsilon());
	boost::for_each(row, [&flat_ctxt](const surface_pair & pair) {
		pair.set_2d_context(&flat_ctxt);
	});

	typedef CGAL::Envelope_diagram_2<Surface_pair_envelope_traits> env_diag;
	env_diag diag;
	CGAL::lower_envelope_3(row.begin(), row.end(), diag);

	const oriented_area & base = surf_rels[base_index][base_index].base();

	for (auto p = diag.faces_begin(); p != diag.faces_end(); ++p) {
		if (!p->is_unbounded()) {
			polygon_2 this_poly;
			auto ccb = p->outer_ccb();
			auto end = ccb;
			CGAL_For_all(ccb, end) {
				this_poly.push_back(result_ctxt->snap(ccb->target()->point()));
			}
			// for some reason the envelope calculation creates degenerate 
			// faces sometimes
			if (!geometry_common::cleanup_loop(&this_poly, EPS_MAGIC)) {
				continue;
			}
			// we have to do two passes because of a bug in 
			// geometry_common::cleanup_loop. see issue #4
			// update: should be resolved but i haven't gotten around to 
			// testing this particular path with this removed
			if (!geometry_common::cleanup_loop(&this_poly, EPS_MAGIC)) {
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