#include "precompiled.h"

#include "exceptions.h"
#include "report.h"
#include "sbt-core.h"
#include "space.h"
#include "surface.h"

#include "convert_to_space_boundaries.h"

using namespace reporting;

namespace interface_conversion {

namespace impl {

namespace {

void set_vertex(polyloop * loop, size_t ix, double x, double y, double z) {
	loop->vertices[ix].x = x;
	loop->vertices[ix].y = y;
	loop->vertices[ix].z = z;
}

template <typename PointRange>
void set_geometry(space_boundary * sb, const PointRange & geometry) {
	sb->geometry.vertex_count = geometry.size();
	sb->geometry.vertices = (point *)malloc(sizeof(point) * geometry.size());
	size_t i = 0;
	boost::for_each(geometry, [sb, &i](const point_3 & p) {
		set_vertex(&sb->geometry, i++, CGAL::to_double(p.x()), CGAL::to_double(p.y()), CGAL::to_double(p.z()));
	});
}

} // namespace

space_boundary * create_unlinked_space_boundary(
	const surface & s, 
	double output_eps) 
{
	using namespace CGAL;
	typedef boost::format fmt;

	space_boundary * newsb = (space_boundary *)malloc(sizeof(space_boundary));
	if (!newsb) { throw failed_malloc_exception(); }

	bool stack_overflowed = false;

	try {
		strncpy(newsb->global_id, s.guid().c_str(), SB_ID_MAX_LEN);
		strncpy(
			newsb->element_name, 
			s.is_virtual() ? "" : s.bounded_element()->name().c_str(), 
			ELEMENT_NAME_MAX_LEN);

		auto cleaned_geometry = s.geometry().to_3d(true).front().outer();
		if (!geometry_common::cleanup_loop(&cleaned_geometry, output_eps)) {
			free(newsb);
			return nullptr;
		}

		if (s.geometry().sense()) {
			set_geometry(newsb, cleaned_geometry);
		}
		else {
			set_geometry(newsb, cleaned_geometry | boost::adaptors::reversed);
		}
	
		direction_3 norm = 
			s.geometry().sense() ? 
				s.geometry().orientation().direction() : 
				-s.geometry().orientation().direction();
		newsb->normal_x = CGAL::to_double(norm.dx());
		newsb->normal_y = CGAL::to_double(norm.dy());
		newsb->normal_z = CGAL::to_double(norm.dz());

		newsb->opposite = nullptr;
		newsb->parent = nullptr;
		
		equality_context lc(EPS_MAGIC);
	
		newsb->material_layer_count = s.material_layers().size();
		if (newsb->material_layer_count > 0) {
			auto ids_sz = sizeof(element_id_t) * newsb->material_layer_count;
			auto layers_sz = sizeof(double) * newsb->material_layer_count;
			newsb->layers = (element_id_t *)malloc(ids_sz);
			newsb->thicknesses = (double *)malloc(layers_sz);
			if (!newsb->layers || !newsb->thicknesses) { 
				throw failed_malloc_exception(); 
			}
			for (size_t j = 0; j < newsb->material_layer_count; ++j) {
				auto id = s.material_layers()[j].layer_element().material();
				auto thickness = *s.material_layers()[j].thickness();
				newsb->layers[j] = id;
				newsb->thicknesses[j] = to_double(lc.snap_height(thickness));
			}
		}
		else {
			newsb->layers = nullptr;
			newsb->thicknesses = nullptr;
		}

		newsb->bounded_space = s.bounded_space().original_info();
		newsb->is_external = s.is_external();
		newsb->is_virtual = s.is_virtual();
	}
	catch (stack_overflow_exception &) {
		// do as little possible here because the stack is still damaged
		stack_overflowed = true;
	}

	if (stack_overflowed) {
		_resetstkoflw();
		report_warning("Internal error: stack overflow. Please report this SBT"
			           "bug.");
		newsb = nullptr;
	}

	return newsb;
}

} // namespace

} // namespace interface_conversion