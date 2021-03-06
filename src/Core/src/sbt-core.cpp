#include "precompiled.h"

#include "assign_openings.h"
#include "build_blocks.h"
#include "convert_to_space_boundaries.h"
#include "equality_context.h"
#include "exceptions.h"
#include "geometry_common.h"
#include "guid_filter.h"
#include "identify_transmission.h"
#include "load_elements.h"
#include "load_spaces.h"
#include "report.h"
#include "space.h"
#include "surface.h"
#include "transmission_information.h"

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

class translator_setter {
public:
	typedef void (*translator_t)(unsigned int, struct _EXCEPTION_POINTERS *);
	translator_setter(translator_t translator)
		: old_(_set_se_translator(translator))
	{ }
	~translator_setter() { _set_se_translator(old_); }
private:
	translator_t old_;
};

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
	typedef boost::format fmt;

	g_opts = opts;

	if (g_opts.notify_func == NULL) { g_opts.notify_func = &do_nothing; }
	if (g_opts.warn_func == NULL) { g_opts.warn_func = &do_nothing; }
	if (g_opts.error_func == NULL) { g_opts.error_func = &do_nothing; }

	guid_filter element_filter = create_guid_filter(
		g_opts.element_filter, 
		g_opts.element_filter_count);
	guid_filter space_filter = create_guid_filter(
		g_opts.space_filter, 
		g_opts.space_filter_count);

	sbt_return_t retval;

	translator_setter translate(&exception_translator);

	try {
		report_progress(
			fmt("Beginning processing for %u building elements.\n") 
			% element_count);

		equality_context ctxt(g_opts.tolernace_in_meters);
		double height_cutoff =
			g_opts.max_pair_distance_in_meters *
			g_opts.length_units_per_meter;

		std::vector<element> elements = load_elements(
			element_infos, 
			element_count, 
			&ctxt, 
			element_filter);
		std::vector<space> spaces = load_spaces(
			space_infos, 
			space_count, 
			&ctxt, 
			space_filter);
		std::vector<block> blocks = blocking::build_blocks(
			elements,
			&ctxt, 
			height_cutoff);
		std::vector<transmission_information> t_info = 
			traversal::identify_transmission(
				blocks, 
				spaces, 
				height_cutoff, 
				&ctxt);

		std::vector<std::unique_ptr<surface>> surfaces;
		boost::for_each(t_info, [&](const transmission_information & ti) { 
			ti.to_surfaces(std::back_inserter(surfaces)); 
		});

		auto opening_blocks = 
			blocks | boost::adaptors::filtered([](const block & b) { 
				return b.is_fenestration(); 
			});
		opening_assignment::assign_openings(
			&surfaces, 
			opening_blocks, 
			g_opts.length_units_per_meter / 3);

		report_progress(
			"Converting internal structures to interface structures");
		retval = interface_conversion::convert_to_space_boundaries(
			surfaces, 
			space_boundaries, 
			space_boundary_count,
			opts.length_units_per_meter * opts.tolernace_in_meters);
		report_progress("done.\n");
	}
	catch (sbt_exception & ex) {
		retval = ex.code();
	}

	if (retval == SBT_STACK_OVERFLOW) {
		_resetstkoflw();
	}

	return retval;
}

void release_space_boundaries(space_boundary ** sbs, size_t count) {
	for (size_t i = 0; i < count; ++i) {
		free(sbs[i]->geometry.vertices);
		free(sbs[i]);
	}
	free(sbs);
}

sb_calculation_options create_default_options() {
	sb_calculation_options opts;
	opts.flags = SBT_NONE;
	opts.length_units_per_meter = 1.0;
	opts.max_pair_distance_in_meters = 0.5;
	opts.space_filter = nullptr;
	opts.space_filter_count = 0;
	opts.element_filter = nullptr;
	opts.element_filter_count = 0;
	opts.notify_func = nullptr;
	opts.warn_func = nullptr;
	opts.error_func = nullptr;
	return opts;
}