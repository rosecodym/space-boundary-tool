#pragma once

#include "precompiled.h"

#include "exceptions.h"
#include "guid_filter.h"
#include "report.h"
#include "space.h"

inline std::vector<space> load_spaces(space_info ** infos, size_t space_count, equality_context * c, const guid_filter & filter) {
	using namespace reporting;
	typedef boost::format fmt;

	std::vector<space> res;
	for (size_t i = 0; i < space_count; ++i) {
		try {
			if (filter(infos[i]->id)) {
				report_progress(fmt("Loading space %s...") % infos[i]->id);
				res.push_back(space(infos[i], c));
				report_progress("done.\n");
			}
		}
		catch (unsupported_geometry_exception & ex) {
			auto msg = fmt(
				"Space %s has unsupported geometry (%s). It will "
				"be ignored.\n");
			report_warning(msg % infos[i]->id % ex.condition());
		}
		catch (unknown_geometry_rep_exception & /*ex*/) {
			auto msg = fmt(
				"Space %s has an unknown internal geometry representation "
				"type. It will be ignored. Please report this SBT bug.\n");
			report_warning(msg % infos[i]->id);
		}
		catch (bad_geometry_exception & ex) {
			auto msg = 
				fmt("Space %s has bad geometry (%s). It will be ignored.\n");
			reporting::report_warning(msg % infos[i]->id % ex.condition());
		}
	}
	return res;
}