#pragma once

#include "precompiled.h"

#include "sbt-core.h"

#define FLAGGED(flag) (g_opts.flags & (flag))

#define NOTIFY_MSG(...) \
	do { \
		sprintf(g_msgbuf, __VA_ARGS__); \
		g_opts.notify_func(g_msgbuf); \
	} \
	while (false);

#define ERROR_MSG(...) \
	do { \
		sprintf(g_msgbuf, __VA_ARGS__); \
		g_opts.notify_func(g_msgbuf); \
	} \
	while (false);

#define PRINT_IF_FLAGGED(flag, ...) \
	do { \
		if (FLAGGED(flag)) { \
			NOTIFY_MSG(__VA_ARGS__); \
		} \
	} \
	while (false);

#define PRINT_POINT_2(p) NOTIFY_MSG("(%f, %f)", CGAL::to_double(p.x()), CGAL::to_double(p.y()));
#define PRINT_POINT_3(p) NOTIFY_MSG("(%f, %f, %f)", CGAL::to_double(p.x()), CGAL::to_double(p.y()), CGAL::to_double(p.z()));
#define PRINT_DIRECTION(d) NOTIFY_MSG("<%f, %f, %f>", CGAL::to_double(d.dx()), CGAL::to_double(d.dy()), CGAL::to_double(d.dz()));
#define PRINT_VECTOR_3(v) NOTIFY_MSG("<%f, %f, %f>", CGAL::to_double(v.x()), CGAL::to_double(v.y()), CGAL::to_double(v.z()));
#define PRINT_POLYGON(poly) std::for_each((poly).vertices_begin(), (poly).vertices_end(), [](const point_2 & p) { PRINT_POINT_2(p); NOTIFY_MSG("\n"); });
#define PRINT_LOOP_2(loop) boost::for_each(loop, [](const point_2 & p) { PRINT_POINT_2(p); NOTIFY_MSG("\n"); });
#define PRINT_LOOP_3(loop) boost::for_each(loop, [](const point_3 & p) { PRINT_POINT_3(p); NOTIFY_MSG("\n"); });

#define PRINT_ELEMENTS(...) PRINT_IF_FLAGGED(SBT_VERBOSE_ELEMENTS, __VA_ARGS__)
#define PRINT_BLOCKS(...) PRINT_IF_FLAGGED(SBT_VERBOSE_BLOCKS, __VA_ARGS__)
#define PRINT_STACKS(...) PRINT_IF_FLAGGED(SBT_VERBOSE_STACKS, __VA_ARGS__)
#define PRINT_SPACES(...) PRINT_IF_FLAGGED(SBT_VERBOSE_SPACES, __VA_ARGS__)
#define PRINT_SOLIDS(...) PRINT_IF_FLAGGED((SBT_VERBOSE_ELEMENTS | SBT_VERBOSE_SPACES), __VA_ARGS__)
#define PRINT_2D_OPERATIONS(...) PRINT_IF_FLAGGED((SBT_VERBOSE_BLOCKS | SBT_VERBOSE_STACKS), __VA_ARGS__)

extern sb_calculation_options g_opts;
extern char g_msgbuf[256];