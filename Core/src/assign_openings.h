#pragma once

#include "precompiled.h"

#include "surface.h"
#include "printing-macros.h"

namespace opening_assignment {

template <typename SurfaceRange>
void assign_openings(SurfaceRange * surfaces, double height_eps) {
	if (std::distance(surfaces->begin(), surfaces->end()) > 0) {
		NOTIFY_MSG("Assigning openings");
		for (auto chld = surfaces->begin(); chld != surfaces->end(); ++chld) {
			if ((*chld)->is_fenestration()) {
				for (auto prnt = surfaces->begin(); prnt != surfaces->end(); ++prnt) {
					// note that this might fail if the parent has been split due to a void or concavity
					// this situation will be astonishingly rare in the wild, but it's there
					if (chld != prnt && !(*prnt)->is_fenestration() && (*chld)->set_parent_maybe(prnt->get(), height_eps)) {
						break;
					}
				}
				NOTIFY_MSG(".");
			}
		}
		NOTIFY_MSG("done.\n");
	}
	else {
		NOTIFY_MSG("No openings to assign.\n");
	}
}

} // namespace opening_assignment