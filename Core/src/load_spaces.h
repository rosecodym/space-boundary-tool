#pragma once

#include "precompiled.h"

#include "element.h"
#include "printing-macros.h"
#include "space.h"

template <typename ElementRange>
std::vector<space> load_spaces(space_info ** infos, size_t space_count, const ElementRange & elements, equality_context * c) {
	std::vector<space> res;
	std::transform(infos, infos + space_count, std::back_inserter(res), [&elements, c](space_info * s) -> space {
		NOTIFY_MSG("Loading space %s...", s->id);
		space sp(s, c);
		NOTIFY_MSG("done.\n");
		return sp;
	});
	return res;
}