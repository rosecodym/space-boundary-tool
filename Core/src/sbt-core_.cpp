#include "precompiled.h"

#include "block.h"
#include "build_blocks.h"
#include "guid_filter.h"
#include "element.h"
#include "load_spaces.h"
#include "load_elements_.h"
#include "printing-macros.h"
#include "sbt-core.h"
#include "space.h"

extern sb_calculation_options g_opts;
extern char g_msgbuf[256];

namespace {

void do_nothing(char * /*msg*/) { }

} // namespace

sbt_return_t calculate_space_boundaries_(
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

	sbt_return_t retval = SBT_OK;

	try {
		equality_context whole_building_context(g_opts.equality_tolerance);

		auto element_filter = create_guid_filter(g_opts.element_filter, g_opts.element_filter_count);
	
		std::vector<element> elements = load_elements(element_infos, element_count, &whole_building_context, element_filter);
		NOTIFY_MSG("Loaded %u elements.\n", elements.size());

		std::vector<space> spaces = load_spaces(space_infos, space_count, elements, &whole_building_context);
		NOTIFY_MSG("Loaded %u spaces.\n", spaces.size());

		std::vector<block> blocks = build_blocks(elements, &whole_building_context);
		NOTIFY_MSG("Built %u blocks.\n", blocks.size());
	}
	catch (std::exception & ex) {
		NOTIFY_MSG(ex.what());
	}

	*space_boundary_count = 0;
	*space_boundaries = nullptr;

	return retval;
}