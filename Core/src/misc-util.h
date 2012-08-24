#pragma once

#include "precompiled.h"

#include "CreateGuid_64.h"
#include "sbt-core.h"

#define FLAGGED(flag) (g_opts.flags & (flag))

#define IF_FLAGGED(flag) if (FLAGGED(flag))
#define IF_NOT_FLAGGED(flag) if (!FLAGGED(flag))

extern sb_calculation_options g_opts;
extern char g_msgbuf[256];

namespace util {

namespace misc {

inline std::string new_guid_as_string() {
	char buf[24] = "ERROR_MSG CREATING GUID";
	CreateCompressedGuidString(buf, 23);
	return std::string(buf);
}

inline bool space_passes_filter(const std::string & guid, const sb_calculation_options opts) {
	if (opts.space_filter_count == 0) { return true; }
	for (size_t i = 0; i < opts.space_filter_count; ++i) { if (std::string(opts.space_filter[i]) == guid) { return true; } }
	return false;
}

inline bool element_passes_filter(const std::string & guid, const sb_calculation_options opts) {
	if (opts.element_filter_count == 0) { return true; }
	for (size_t i = 0; i < opts.element_filter_count; ++i) { if (std::string(opts.element_filter[i]) == guid) { return true; } }
	return false;
}

template <class InputRange, class OutputIterator>
OutputIterator unpack_loop_extrusion(InputRange base, InputRange target, OutputIterator oi) {
	std::vector<std::deque<typename InputRange::value_type>> res(base.size());
	for (size_t i = 0; i < base.size(); ++i) {
		res[i].push_back(target[i]);
		res[i].push_back(base[i]);
		res[(i+1) % base.size()].push_front(target[i]);
		res[(i+1) % base.size()].push_front(base[i]);
	}
	std::move(res.begin(), res.end(), oi);
	return oi;
}

} // namespace util

} // namespace misc