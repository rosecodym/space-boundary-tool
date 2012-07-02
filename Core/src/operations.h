#pragma once

#include "precompiled.h"

#include "element.h"
#include "guid_filter.h"
#include "space.h"

struct element_info;
struct space_info;
struct space_boundary;
class surface;
class equality_context;

namespace operations {

std::vector<std::shared_ptr<space>>		extract_spaces(space_info ** infos, size_t count, std::shared_ptr<equality_context> context, std::function<point_3(point_3)> corrector, const guid_filter & filter);
std::vector<std::shared_ptr<surface>>	build_blocks(const std::vector<std::shared_ptr<element>> & elements, std::shared_ptr<equality_context> context);
std::vector<std::shared_ptr<surface>>	build_stacks(const std::vector<std::shared_ptr<surface>> & blocked_surfaces, const std::vector<std::shared_ptr<space>> & spaces, std::shared_ptr<equality_context> & c);
std::vector<std::shared_ptr<surface>>	assign_openings(const std::vector<std::shared_ptr<surface>> & surfaces);
std::vector<std::shared_ptr<surface>>	attach_to_spaces(const std::vector<std::shared_ptr<surface>> & surfaces, const std::vector<std::shared_ptr<space>> & spaces);
std::vector<std::shared_ptr<surface>>	resolve_levels(const std::vector<std::shared_ptr<surface>> & assigned_surfaces);

}