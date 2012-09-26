#pragma once

#include "precompiled.h"

#include "sbt-core.h"

extern sb_calculation_options g_opts;

namespace reporting {

inline void report_progress(const boost::format & fmt) {
	g_opts.notify_func(const_cast<char *>(fmt.str().c_str()));
}

inline void report_progress(const char * msg) {
	g_opts.notify_func(const_cast<char *>(msg));
}

inline void report_warning(const boost::format & fmt) {
	g_opts.warn_func(const_cast<char *>(fmt.str().c_str()));
}

inline void report_warning(const char * msg) {
	g_opts.warn_func(const_cast<char *>(msg));
}

inline void report_error(const boost::format & fmt) {
	g_opts.error_func(const_cast<char *>(fmt.str().c_str()));
}

inline void report_error(const char * msg) {
	g_opts.error_func(const_cast<char *>(msg));
}

} // namespace reporting