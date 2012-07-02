#pragma once

#include "precompiled.h"

struct solid;
struct extruded_area_solid;

#include "cgal-typedefs.h"

void handle_easy_ext_case(
	const extruded_area_solid & ext, 
	const vector_3 & vec, 
	double thicknesses[], 
	int layer_count, 
	solid results[]);
