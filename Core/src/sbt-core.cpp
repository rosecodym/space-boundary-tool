#include "precompiled.h"

#include "assign_openings.h"
#include "build_blocks.h"
#include "build_stacks.h"
#include "convert_to_space_boundaries.h"
#include "equality_context.h"
#include "exceptions.h"
#include "geometry_common.h"
#include "guid_filter.h"
#include "load_elements.h"
#include "load_spaces.h"
#include "misc-util.h"
#include "report.h"
#include "sbt-core-helpers.h"
#include "space.h"
#include "surface.h"

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

using namespace reporting;

namespace {

void do_nothing(char * /*msg*/) { }

void exception_translator(unsigned int code, struct _EXCEPTION_POINTERS *) {
	if (code == EXCEPTION_STACK_OVERFLOW) {
		// don't call _resetskoflow() here yet - the stack isn't unwound. or something.
		throw stack_overflow_exception();
	}
	else {
		throw sbt_exception();
	}
}

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

	_set_se_translator(&exception_translator);

	try {
		report_progress(boost::format("Beginning processing for %u building elements.\n") % element_count);

		equality_context whole_building_context(g_opts.equality_tolerance);

		std::vector<element> elements = load_elements(element_infos, element_count, &whole_building_context, element_filter);
		std::vector<space> spaces = load_spaces(space_infos, space_count, &whole_building_context, space_filter);
		std::vector<block> blocks = blocking::build_blocks(elements, &whole_building_context);
		std::vector<blockstack> stacks = stacking::build_stacks(blocks, spaces, g_opts.max_pair_distance, &whole_building_context);

		std::vector<std::unique_ptr<surface>> surfaces;
		boost::for_each(stacks, [&surfaces](const blockstack & st) { st.to_surfaces(std::back_inserter(surfaces)); });

		opening_assignment::assign_openings(&surfaces, g_opts.equality_tolerance);

		report_progress("Converting internal structures to interface structures");
		retval = interface_conversion::convert_to_space_boundaries(surfaces, space_boundaries, space_boundary_count);
		report_progress("done.\n");
	}
	catch (sbt_exception & ex) {
		retval = ex.code();
	}

	if (retval == SBT_TOO_COMPLICATED) {
		_resetstkoflw();
	}

	return retval;
}