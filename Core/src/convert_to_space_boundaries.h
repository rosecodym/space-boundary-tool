#pragma once

#include "precompiled.h"

#include "cleanup_loop.h"
#include "exceptions.h"
#include "report.h"
#include "sbt-core.h"
#include "sbt-core-helpers.h"
#include "surface.h"

extern sb_calculation_options g_opts;

namespace interface_conversion {

namespace impl {

space_boundary * create_unlinked_space_boundary(const surface & surf);

} // namespace impl

template <typename SurfaceRange>
sbt_return_t convert_to_space_boundaries(const SurfaceRange & surfaces, space_boundary *** sbs, size_t * sb_count) {
	std::map<std::string, space_boundary *> boundaries;

	try {
		// create unlinked structures
		boost::copy(surfaces
			| boost::adaptors::transformed([](const std::unique_ptr<surface> & surf) -> std::pair<std::string, space_boundary *> { 
				reporting::report_progress(".");
				return std::make_pair(surf->guid(), impl::create_unlinked_space_boundary(*surf.get())); 
			})
			| boost::adaptors::filtered([](const std::pair<std::string, space_boundary *> & entry) { return entry.second != nullptr; }),
			std::inserter(boundaries, boundaries.begin()));

		// link to opposites
		boost::for_each(surfaces 
			| boost::adaptors::filtered([&boundaries](const std::unique_ptr<surface> & surf) {
				return surf->has_other_side() && boundaries.find(surf->guid()) != boundaries.end();
			}),
			[&boundaries](const std::unique_ptr<surface> & surf) {
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
			| boost::adaptors::filtered([&boundaries](const std::unique_ptr<surface> & surf) {
				return surf->parent() && boundaries.find(surf->guid()) != boundaries.end();
			}),
			[&boundaries](const std::unique_ptr<surface> & surf) {
				auto matching_parent = boundaries.find(surf->parent()->guid());
				if (matching_parent != boundaries.end()) {
					boundaries[surf->guid()]->parent = matching_parent->second;
				}
			});

		// set levels
		boost::for_each(surfaces, [&boundaries](const std::unique_ptr<surface> & surf) {
			auto matching_structure = boundaries.find(surf->guid());
			if (matching_structure != boundaries.end()) {
				matching_structure->second->level =
					surf->is_virtual() ?					2 :
					surf->is_fenestration() ?				2 :
					surf->is_external() ?					2 :
					surf->shares_space_with_other_side() ?	4 :
					surf->has_other_side() ?				2 : 5;
			}
		});

		(*sbs) = (space_boundary **)malloc(sizeof(space_boundary *) * boundaries.size());
		if (!*sbs) { throw failed_malloc_exception(); }
		boost::copy(boost::adaptors::values(boundaries), *sbs);
		*sb_count = boundaries.size();
		return SBT_OK;
	}
	catch (failed_malloc_exception &) {
		reporting::report_warning("An allocation failed while generating interface structures! Try simplifying the building to reduce the final space boundary count. SBT should be restarted.\n");
		free(*sbs);
		boost::for_each(boost::adaptors::values(boundaries), [](space_boundary * sb) { free(sb); });
		return SBT_FAILED_ALLOCATION;
	}
}

} // namespace interface_conversion