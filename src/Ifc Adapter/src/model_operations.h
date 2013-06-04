#pragma once

#include "precompiled.h"
#include "number_collection.h"
#include "sbt-ifcadapter.h"

class unit_scaler;

ifcadapter_return_t extract_from_model(
	cppw::Open_model & model, 
	size_t * element_count, 
	element_info *** elements, 
	double ** composite_layer_dxs,
	double ** composite_layer_dys,
	double ** composite_layer_dzs,
	size_t * space_count,
	space_info *** spaces,
	void (*notify)(char *),
	void (*warn)(char *),
	const std::function<bool(const char *)> & element_filter,
	const std::function<bool(const char *)> & space_filter,
	number_collection<K> * c,
	std::vector<element_info *> * shadings);

ifcadapter_return_t add_to_model(
	cppw::Open_model & model,
	size_t sb_count,
	space_boundary ** space_boundaries,
	void (*msg_func)(char *),
	number_collection<iK> * c);