#pragma once

#include "precompiled.h"

#include "sbt-ifcadapter.h"

inline void reverse_points(polyloop * loop) {
	std::reverse(loop->vertices, loop->vertices + loop->vertex_count);
}