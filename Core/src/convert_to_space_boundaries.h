#pragma once

#include "precompiled.h"

#include "cleanup_loop.h"
#include "exceptions.h"
#include "report.h"
#include "sbt-core.h"
#include "surface.h"

extern sb_calculation_options g_opts;

namespace interface_conversion {

namespace impl {

space_boundary * create_unlinked_space_boundary(const surface & surf);

} // namespace impl

template <typename SurfaceRange>
sbt_return_t convert_to_space_boundaries(const SurfaceRange & surfaces, space_boundary *** sbs, size_t * sb_count) {
	typedef std::unique_ptr<surface> surface_ptr;
	typedef std::pair<std::string, space_boundary *> interfc_entry;

	std::map<std::string, space_boundary *> boundaries;

	int max_dots = 60;
	int per_dot = surfaces.size() / max_dots;
	if (per_dot == 0) { per_dot = 1; }
	int curr_count = 0;
	auto tick = [per_dot, &curr_count]() {
		if (++curr_count % per_dot == 0) {
			reporting::report_progress(".");
		}
	};

	using namespace boost::adaptors;

	try {
		std::vector<space_boundary *> unlinked;
		for (auto s = surfaces.begin(); s != surfaces.end(); ++s) {
			auto unlinked = impl::create_unlinked_space_boundary(*s->get());
			if (unlinked != nullptr) {
				boundaries[(*s)->guid()] = unlinked;
			}
			tick();
		}

		// link to opposites
		boost::for_each(surfaces 
			| filtered([&boundaries](const surface_ptr & surf) {
				return surf->has_other_side() && boundaries.find(surf->guid()) != boundaries.end();
			}),
			[&boundaries](const surface_ptr & surf) {
				auto matching_interface_structure = boundaries.find(surf->guid())->second;
				if (matching_interface_structure->opposite == nullptr) {
					auto matching_opposite = boundaries.find(surf->other_side()->guid());
					if (matching_opposite != boundaries.end()) {
						matching_interface_structure->opposite = matching_opposite->second;
						matching_opposite->second->opposite = matching_interface_structure;
						return;
					}
				}
			});

		// link to parents
		boost::for_each(surfaces
			| filtered([&boundaries](const surface_ptr & surf) {
				return surf->parent() && boundaries.find(surf->guid()) != boundaries.end();
			}),
			[&boundaries](const surface_ptr & surf) {
				auto matching_parent = boundaries.find(surf->parent()->guid());
				if (matching_parent != boundaries.end()) {
					boundaries[surf->guid()]->parent = matching_parent->second;
				}
			});

		(*sbs) = (space_boundary **)malloc(sizeof(space_boundary *) * boundaries.size());
		if (!*sbs) { throw failed_malloc_exception(); }
		boost::copy(values(boundaries), *sbs);
		*sb_count = boundaries.size();
		return SBT_OK;
	}
	catch (failed_malloc_exception &) {
		reporting::report_error("An allocation failed while generating interface structures! Try simplifying the building to reduce the final space boundary count. SBT should be restarted.\n");
		free(*sbs);
		boost::for_each(values(boundaries), [](space_boundary * sb) { free(sb); });
		return SBT_FAILED_ALLOCATION;
	}
}

} // namespace interface_conversion