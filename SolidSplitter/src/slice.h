#pragma once

#include "precompiled.h"

#include "cgal-typedefs.h"

struct solid;
class equality_context;

void slice(
	const solid & s, 
	const vector_3 & vec,
	double thicknesses[], 
	int layer_count, 
	solid results[],
	equality_context * c);