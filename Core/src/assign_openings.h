#pragma once

#include "precompiled.h"

#include "report.h"
#include "surface.h"

namespace opening_assignment {

template <typename SurfaceRange>
void assign_openings(SurfaceRange * surfaces, double height_eps) {
	if (std::distance(surfaces->begin(), surfaces->end()) > 0) {
		reporting::report_progress("Assigning openings");
		for (auto chld = surfaces->begin(); chld != surfaces->end(); ++chld) {
			if ((*chld)->is_fenestration()) {
				for (auto prnt = surfaces->begin(); prnt != surfaces->end(); ++prnt) {
					// note that this might fail if the parent has been split due to a void or concavity
					// this situation will be astonishingly rare in the wild, but it's there
					if (chld != prnt && !(*prnt)->is_fenestration() && (*chld)->set_parent_maybe(prnt->get(), height_eps)) {
						break;
					}
				}
				reporting::report_progress(".");
			}
		}
		reporting::report_progress("done.\n");
	}
	else {
		reporting::report_progress("No openings to assign.\n");
	}
}

} // namespace opening_assignment