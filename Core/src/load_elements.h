#pragma once

#include "precompiled.h"

#include "element.h"
#include "guid_filter.h"

struct element_info;
class equality_context;

std::vector<element> load_elements(element_info ** infos, size_t count, equality_context * c, const guid_filter & filter);