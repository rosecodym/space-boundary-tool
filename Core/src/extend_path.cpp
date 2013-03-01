#include "precompiled.h"

#include "extend_path.h"

#include "area.h"
#include "bg_path.h"
#include "transmission_information.h"
#include "vertex_wrapper.h"

namespace traversal {

namespace impl {

void extend_path(
	const bg_path & so_far, 
	const geometry_2d::area & transmission_area,
	const orientation * o,
	double max_thickness,
	const std::function<void(transmission_information)> & save_traversal)
{
	assert(!transmission_area.is_empty());

	if (so_far.length() == 0) {
		boost::for_each(so_far.end_vertex().adjacent(), [&](vertex_wrapper u) {
			geometry_2d::area new_a = transmission_area * u.vertex_area();
			extend_path(
				so_far.with_appended(u), 
				new_a, 
				o, 
				max_thickness, 
				save_traversal);
		});
	}
	else {
		area unaccounted_for(transmission_area);
		auto relevant_adj = so_far.end_vertex().adjacent(so_far.last_edge_h());
		for (auto v = relevant_adj.begin(); v != relevant_adj.end(); ++v) {
			unaccounted_for -= v->vertex_area();
			auto new_area = transmission_area * v->vertex_area();
			if (new_area.is_empty()) {
				// Do nothing - no transmission area
				continue;
			}
			space_face * as_space_face;
			if (v->is_halfblock() ||
				(so_far.total_thickness() + v->thickness() > max_thickness))
			{
				// Non-transmitting due to halfblock or thickness
				save_traversal(transmission_information(
					new_area,
					so_far.with_appended(*v).to_layers(),
					so_far.start_face()->sense(),
					o,
					false,
					so_far.start_face()->bounded_space(),
					so_far.start_face()->height()));
			}
			else if ((as_space_face = v->as_space_face()) != nullptr) {
				// Transmitting, internal
				save_traversal(transmission_information(
					new_area,
					so_far.with_appended(*v).to_layers(),
					so_far.start_face()->sense(),
					o,
					false,
					so_far.start_face()->bounded_space(),
					so_far.start_face()->height(),
					as_space_face->bounded_space(),
					as_space_face->height()));
				as_space_face->remove_area(new_area);
			}
			else {
				// Keep looking
				extend_path(
					so_far.with_appended(*v),
					new_area,
					o,
					max_thickness,
					save_traversal);
			}
		}
		if (!unaccounted_for.is_empty()) {
			// Transmitting, external
			save_traversal(transmission_information(
				unaccounted_for,
				so_far.to_layers(),
				so_far.start_face()->sense(),
				o,
				true,
				so_far.start_face()->bounded_space(),
				so_far.start_face()->height()));
		}
	}
}

} // namespace impl

} // namespace traversal