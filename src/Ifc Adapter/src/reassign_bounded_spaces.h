#pragma once

#include "precompiled.h"

struct space_info;
struct space_boundary;

void reassign_bounded_spaces(
	size_t loaded_space_count,
	space_info ** loaded_space_info,
	size_t calculated_space_count, 
	space_info ** calculated_space_info,
	size_t sb_count,
	space_boundary ** sbs);