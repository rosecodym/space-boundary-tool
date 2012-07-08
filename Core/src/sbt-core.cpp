#include "precompiled.h"

#include "build_blocks.h"
#include "core_exception.h"
#include "equality_context.h"
#include "geometry_common.h"
#include "guid_filter.h"
#include "load_elements.h"
#include "operations.h"
#include "sbt-core-helpers.h"
#include "space.h"
#include "surface.h"
#include "misc-util.h"
#include "printing-util.h"

#include "sbt-core.h"

sb_calculation_options g_opts;
char g_msgbuf[256];

sbt_return_t calculate_space_boundaries_(
	size_t element_count,
	element_info ** element_infos,
	size_t space_count,
	space_info ** space_infos,
	size_t * space_boundary_count,
	space_boundary *** space_boundaries,
	sb_calculation_options opts);

namespace {

template <typename PointRange>
void set_geometry(space_boundary * sb, const PointRange & geometry) {
	set_vertex_count(&sb->geometry, geometry.size());
	size_t i = 0;
	boost::for_each(geometry, [sb, &i](const point_3 & p) {
		set_vertex(&sb->geometry, i++, CGAL::to_double(p.x()), CGAL::to_double(p.y()), CGAL::to_double(p.z()));
	});
}
	
sbt_return_t convert_to_space_boundaries(
	const std::vector<std::shared_ptr<surface>> & surfaces, 
	const std::vector<std::shared_ptr<space>> & /*spaces*/, 
	space_boundary *** sbs,
	space_info ** /*space_infos*/)
{
	if (FLAGGED(SBT_EXPENSIVE_CHECKS)) {
		boost::for_each(surfaces, [](const std::shared_ptr<surface> & s) {
			if (s->get_level() != 5 && s->opposite().expired()) {
				ERROR_MSG("Space boundary %s/%s is level %i but has no opposite space boundary.\n",
					s->guid().c_str(),
					s->get_space().lock()->global_id().c_str(),
					s->get_level());
				abort();
			}
		});
	}

	*sbs = (space_boundary **)malloc(sizeof(space_boundary *) * surfaces.size());
	for (size_t i = 0; i < surfaces.size(); ++i) {
		std::shared_ptr<surface> surf = surfaces[i];
		space_boundary * newsb = (*sbs)[i] = (space_boundary *)malloc(sizeof(space_boundary));
		newsb->geometry.vertex_count = 0;

		strncpy(newsb->global_id, surf->guid().c_str(), SB_ID_MAX_LEN);
		strncpy(newsb->element_id, surf->is_virtual() ? "" : surf->element_id().c_str(), ELEMENT_ID_MAX_LEN);

		auto cleaned_geometry = surf->geometry().to_3d().front().outer();
		bool could_clean = geometry_common::cleanup_loop(&cleaned_geometry, g_opts.equality_tolerance);
		if (FLAGGED(SBT_EXPENSIVE_CHECKS) && !could_clean) {
			ERROR_MSG("Couldn't clean up a space boundary loop:\n");
			PRINT_LOOP_3(cleaned_geometry);
			abort();
		}

		if (surf->geometry().sense()) {
			set_geometry(newsb, cleaned_geometry);
		}
		else {
			set_geometry(newsb, cleaned_geometry | boost::adaptors::reversed);
		}
	
		direction_3 norm = surf->geometry().sense() ? surf->geometry().orientation().direction() : -surf->geometry().orientation().direction();
		newsb->normal_x = CGAL::to_double(norm.dx());
		newsb->normal_y = CGAL::to_double(norm.dy());
		newsb->normal_z = CGAL::to_double(norm.dz());

		newsb->opposite = nullptr;
		newsb->parent = nullptr;

		equality_context layers_context(g_opts.equality_tolerance);
	
		newsb->material_layer_count = surf->materials().size();
		if (newsb->material_layer_count > 0) {
			newsb->layers = (material_id_t *)malloc(sizeof(material_id_t) * newsb->material_layer_count);
			newsb->thicknesses = (double *)malloc(sizeof(double) * newsb->material_layer_count);
			for (size_t j = 0; j < newsb->material_layer_count; ++j) {
				newsb->layers[j] = surf->materials()[j].first;
				newsb->thicknesses[j] = CGAL::to_double(layers_context.snap_height(surf->materials()[j].second));
			}
		}
		else {
			newsb->layers = nullptr;
			newsb->thicknesses = nullptr;
		}

		newsb->bounded_space = surf->get_space().lock()->original_info();
		newsb->lies_on_outside = surf->lies_on_outside();
		newsb->level = surf->get_level();
		newsb->is_virtual = surf->is_virtual();

		NOTIFY_MSG(".");
	}

	for (size_t i = 0; i < surfaces.size(); ++i) {
		if (!surfaces[i]->opposite().expired() && (*sbs)[i]->opposite == nullptr) {
			for (size_t j = i + 1; j < surfaces.size(); ++j) {
				if (surfaces[j].get() == surfaces[i]->opposite().lock().get()) {
					(*sbs)[i]->opposite = (*sbs)[j];
					(*sbs)[j]->opposite = (*sbs)[i];
					break;
				}
			}
		}
		if (!surfaces[i]->containing_boundary().expired() && (*sbs)[i]->parent == nullptr) {
			for (size_t j = 0; j < surfaces.size(); ++j) {
				if (surfaces[i]->containing_boundary().lock().get() == surfaces[j].get()) {
					(*sbs)[i]->parent = (*sbs)[j];
				}
			}
		}
	}

	if (FLAGGED(SBT_EXPENSIVE_CHECKS)) {
		std::for_each(*sbs, *sbs + surfaces.size(), [](space_boundary * s) {
			if (s->opposite == nullptr && s->level != 5) {
				ERROR_MSG("Space boundary %s/%x is level %i but has no opposite space boundary.\n",
					s->global_id,
					s->bounded_space,
					s->level);
				abort();
			}
		});
	}

	return SBT_OK;
}

void do_nothing(char * /*msg*/) { }

} // namespace

sbt_return_t calculate_space_boundaries(
	size_t element_count,
	element_info ** element_infos,
	size_t space_count,
	space_info ** space_infos,
	size_t * space_boundary_count,
	space_boundary *** space_boundaries,
	sb_calculation_options opts)
{

	g_opts = opts;

	if (g_opts.notify_func == NULL) { g_opts.notify_func = &do_nothing; }
	if (g_opts.warn_func == NULL) { g_opts.warn_func = &do_nothing; }
	if (g_opts.error_func == NULL) { g_opts.error_func = &do_nothing; }

	guid_filter element_filter = create_guid_filter(g_opts.element_filter, g_opts.element_filter_count);
	guid_filter space_filter = create_guid_filter(g_opts.space_filter, g_opts.space_filter_count);

	sbt_return_t retval;
	
	try {

		NOTIFY_MSG("Beginning processing for %u building elements.\n", element_count);
		if (g_opts.flags & SBT_SKIP_WALL_SLAB_CHECK) {
			NOTIFY_MSG("Assuming no wall/slab intersections.\n");
		}
		if (g_opts.flags & SBT_SKIP_WALL_COLUMN_CHECK) {
			NOTIFY_MSG("Assuming no wall/column intersections.\n");
		}
		if (g_opts.flags & SBT_SKIP_SLAB_COLUMN_CHECK) {
			NOTIFY_MSG("Assuming no slab/column intersections.\n");
		}

		std::shared_ptr<equality_context> whole_building_context(new equality_context(g_opts.equality_tolerance));
		std::vector<std::shared_ptr<surface>> surfaces;

		auto corrector = [&whole_building_context](const point_3 & p) -> point_3 { 
			return whole_building_context->snap(p);
		};

		std::vector<std::shared_ptr<element>> elements = load_elements(element_count, element_infos, whole_building_context, corrector, element_filter);

		std::vector<std::shared_ptr<space>> spaces = operations::extract_spaces(space_infos, space_count, whole_building_context, corrector, space_filter);

		std::vector<element> elements_derefed;
		boost::copy(elements | boost::adaptors::indirected, std::back_inserter(elements_derefed));

		auto blocks = blocking::build_blocks(elements_derefed, whole_building_context.get());
		boost::for_each(blocks, [&surfaces](const block & b) { b.as_surfaces(std::back_inserter(surfaces)); });

		NOTIFY_MSG("Beginning stack construction for %u surfaces.\n", surfaces.size());
		surfaces = operations::build_stacks(surfaces, spaces, whole_building_context);
		NOTIFY_MSG("Stack construction complete. %u surface(s) generating from stacking.\n", surfaces.size());

		if (std::find_if(surfaces.begin(), surfaces.end(), [](std::shared_ptr<surface> s) { return s->is_fenestration(); }) != surfaces.end()) {
			NOTIFY_MSG("Assigning openings");
			surfaces = operations::assign_openings(surfaces);
			NOTIFY_MSG("done.\n");
		}
		else {
			NOTIFY_MSG("No openings to assign.\n");
		}

		NOTIFY_MSG("Resolving space boundary levels");
		surfaces = operations::resolve_levels(surfaces);
		NOTIFY_MSG("done.\n");

		NOTIFY_MSG("Converting internal structures to interface structures");
		retval = convert_to_space_boundaries(surfaces, spaces, space_boundaries, space_infos);
		*space_boundary_count = surfaces.size();
		NOTIFY_MSG("done.\n");
	}

	catch (core_exception & e) {
		retval = e.code();
	}

	return retval;
}