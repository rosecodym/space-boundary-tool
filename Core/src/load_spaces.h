#pragma once

#include "precompiled.h"

#include "exceptions.h"
#include "guid_filter.h"
#include "report.h"
#include "space.h"

inline std::vector<space> load_spaces(space_info ** infos, size_t space_count, equality_context * c, const guid_filter & filter) {
	std::vector<space> res;
	for (size_t i = 0; i < space_count; ++i) {
		try {
			if (filter(infos[i]->id)) {
				reporting::report_progress(boost::format("Loading space %s...") % infos[i]->id);
				res.push_back(space(infos[i], c));
				reporting::report_progress("done.\n");
			}
		}
		catch (unsupported_geometry_exception & ex) {
			reporting::report_warning(boost::format("Space %s has unsupported geometry (%s). It will be ignored.\n") % infos[i]->id % ex.condition());
		}
		catch (unknown_geometry_rep_exception & /*ex*/) {
			reporting::report_warning(boost::format("Element %s has unknown internal geometry represetnation type. It will be ignored. Please report this SBT bug.\n") % infos[i]->id);
		}
	}
	return res;
}