#pragma once

#include "precompiled.h"

#include "guid_filter.h"
#include "report.h"
#include "space.h"

std::vector<space> load_spaces(space_info ** infos, size_t space_count, equality_context * c, const guid_filter & filter) {
	std::vector<space> res;
	boost::transform(
		boost::make_iterator_range(infos, infos + space_count) |
			boost::adaptors::filtered([&filter](space_info * sp) -> bool { 
				if (!sp->id || !filter(sp->id)) { return false; }
				else if (sp->geometry.rep_type == REP_BREP && sp->geometry.rep.as_brep.face_count < 4) {
					reporting::report_warning(boost::format("Warning - skipping space %s, as it only has %u face(s).\n") % sp->id % sp->geometry.rep.as_brep.face_count);
					return false;
				}
				else { return true; }
			}),
		std::back_inserter(res),
		[c](space_info * s) -> space {
			reporting::report_progress(boost::format("Loading space %s...") % s->id);
			space sp(s, c);
			reporting::report_progress("done.\n");
			return sp;
		});
	return res;
}