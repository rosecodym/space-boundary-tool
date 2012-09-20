#include "precompiled.h"

#include "exceptions.h"
#include "printing-macros.h"
#include "sbt-core.h"
#include "space.h"
#include "surface.h"

#include "convert_to_space_boundaries.h"

namespace interface_conversion {

namespace impl {

namespace {

template <typename PointRange>
void set_geometry(space_boundary * sb, const PointRange & geometry) {
	set_vertex_count(&sb->geometry, geometry.size());
	size_t i = 0;
	boost::for_each(geometry, [sb, &i](const point_3 & p) {
		set_vertex(&sb->geometry, i++, CGAL::to_double(p.x()), CGAL::to_double(p.y()), CGAL::to_double(p.z()));
	});
}

} // namespace

space_boundary * create_unlinked_space_boundary(const surface & surf) {
	space_boundary * newsb = (space_boundary *)malloc(sizeof(space_boundary));
	if (!newsb) { throw failed_malloc_exception(); }
	newsb->geometry.vertex_count = 0;

	bool stack_overflowed = false;

	try {
		strncpy(newsb->global_id, surf.guid().c_str(), SB_ID_MAX_LEN);
		strncpy(newsb->element_id, surf.is_virtual() ? "" : surf.bounded_element()->source_id().c_str(), ELEMENT_ID_MAX_LEN);

		auto cleaned_geometry = surf.geometry().to_3d(true).front().outer();
		geometry_common::cleanup_loop(&cleaned_geometry, g_opts.equality_tolerance);

		if (surf.geometry().sense()) {
			set_geometry(newsb, cleaned_geometry);
		}
		else {
			set_geometry(newsb, cleaned_geometry | boost::adaptors::reversed);
		}
	
		direction_3 norm = surf.geometry().sense() ? surf.geometry().orientation().direction() : -surf.geometry().orientation().direction();
		newsb->normal_x = CGAL::to_double(norm.dx());
		newsb->normal_y = CGAL::to_double(norm.dy());
		newsb->normal_z = CGAL::to_double(norm.dz());

		newsb->opposite = nullptr;
		newsb->parent = nullptr;equality_context layers_context(g_opts.equality_tolerance);
	
		newsb->material_layer_count = surf.material_layers().size();
		if (newsb->material_layer_count > 0) {
			newsb->layers = (material_id_t *)malloc(sizeof(material_id_t) * newsb->material_layer_count);
			newsb->thicknesses = (double *)malloc(sizeof(double) * newsb->material_layer_count);
			if (!newsb->layers || !newsb->thicknesses) { throw failed_malloc_exception(); }
			for (size_t j = 0; j < newsb->material_layer_count; ++j) {
				newsb->layers[j] = surf.material_layers()[j].layer_element().material();
				newsb->thicknesses[j] = CGAL::to_double(layers_context.snap_height(*surf.material_layers()[j].thickness()));
			}
		}
		else {
			newsb->layers = nullptr;
			newsb->thicknesses = nullptr;
		}

		newsb->bounded_space = surf.bounded_space().original_info();
		newsb->lies_on_outside = false;
		newsb->is_virtual = surf.is_virtual();
	}
	catch (stack_overflow_exception &) {
		// do as little possible here because the stack is still damaged
		stack_overflowed = true;
	}

	if (stack_overflowed) {
		_resetstkoflw();
		if (surf.is_virtual()) {
			WARN_MSG("Warning: a resulting space boundary was too complicated. Try simplifying the connection between space %s and space %s.\n",
				surf.bounded_space().global_id().c_str(),
				surf.other_side()->bounded_space().global_id().c_str());
		}
		else {
			WARN_MSG("Warning: a resulting space boundary was too complicated. Try simplifying the connection between space %s and element %s.\n",
				surf.bounded_space().global_id().c_str(),
				surf.bounded_element()->source_id().c_str());
		} 
		newsb = nullptr;
	}

	return newsb;
}

} // namespace

} // namespace interface_conversion