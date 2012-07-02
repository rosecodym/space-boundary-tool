#pragma once

#include "precompiled.h"

#include "sbt-core.h"

#define NOTIFY_MSG(...) \
	do { \
		sprintf(g_msgbuf, __VA_ARGS__); \
		g_opts.notify_func(g_msgbuf); \
	} \
	while (false);

#define WARN_MSG(...) \
	do { \
		sprintf(g_msgbuf, __VA_ARGS__); \
		g_opts.warn_func(g_msgbuf); \
	} \
	while (false);

#define ERROR_MSG(...) \
	do { \
		sprintf(g_msgbuf, __VA_ARGS__); \
		g_opts.error_func(g_msgbuf); \
	} \
	while (false);

#define ASSERT_FAIL(...) \
	do { \
		sprintf(g_msgbuf, __VA_ARGS__); \
		g_opts.error_func(g_msgbuf); \
		throw core_exception(SBT_ASSERTION_FAILED); \
	} \
	while (false);

extern sb_calculation_options g_opts;
extern char g_msgbuf[256];

namespace util {

namespace printing {

inline void print_dir(void (*msg_func)(char *), const direction_3 & d) {
	sprintf(g_msgbuf, "<%f, %f, %f>", CGAL::to_double(d.dx()), CGAL::to_double(d.dy()), CGAL::to_double(d.dz()));
	msg_func(g_msgbuf);
}

inline void print_polygon(void (*notify_func)(char *), const polygon_2 & poly) {
	for (auto p = poly.vertices_begin(); p != poly.vertices_end(); ++p) {
		sprintf(g_msgbuf, "\t%f\t%f\n", CGAL::to_double(p->x()), CGAL::to_double(p->y()));
		notify_func(g_msgbuf);
	}
}

void print_nef_polygon(void (*msg_func)(char *), const nef_polygon_2 & nef);

} // namespace io

} // namespace util