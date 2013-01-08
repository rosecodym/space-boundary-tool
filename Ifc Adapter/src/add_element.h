#pragma once

#include "precompiled.h"

#include "cgal-typedefs.h"
#include "number_collection.h"
#include "sbt-ifcadapter.h"

class unit_scaler;

void add_element(
	std::vector<element_info *> * infos,
	std::vector<direction_3> * composite_dirs,
	element_type type, 
	const cppw::Instance & inst, 
	void (*msg_func)(char *), 
	void (*warn_func)(char *),
	const unit_scaler & s, 
	int material_id, 
	number_collection<K> * c);