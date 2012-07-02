#pragma once

#include "precompiled.h"

#include "element.h"
#include "guid_filter.h"

struct element_info;
class equality_context;

std::vector<element> load_elements(element_info ** infos, size_t count, equality_context * c, const guid_filter & filter);

// DEPRECATED
inline std::vector<std::shared_ptr<element>> load_elements(
	size_t count, 
	element_info ** elements, 
	std::shared_ptr<equality_context> context, 
	std::function<point_3(point_3)> /*corrector*/, 
	const guid_filter & filter)
{
	std::vector<std::shared_ptr<element>> res;
	boost::transform(load_elements(elements, count, context.get(), filter), std::back_inserter(res), [](const element & e) {
		return std::shared_ptr<element>(new element(e));
	});
	return res;
}