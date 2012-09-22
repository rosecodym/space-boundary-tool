#pragma once

#include "precompiled.h"

#include "number_collection.h"
#include "sbt-ifcadapter.h"

class unit_scaler;

void add_element(std::vector<element_info *> * infos, element_type type, const cppw::Instance & inst, void (*msg_func)(char *), const unit_scaler & s, int material_id, number_collection<K> * c);