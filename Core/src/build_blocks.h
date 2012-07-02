#pragma once

#include "precompiled.h"

#include "element.h"
#include "block.h"

class equality_context;
class surface;

namespace blocking {

namespace impl {

std::vector<block> build_blocks_for(const element & e, equality_context * c);

} // namespace impl

std::vector<block> build_blocks(const std::vector<element> & elements, equality_context * c);

} // namespace blocking

namespace operations {

// DEPRECATED
inline std::vector<std::shared_ptr<surface>> build_blocks(const std::vector<element> & elements, std::shared_ptr<equality_context> & c) {
	std::vector<std::shared_ptr<surface>> res;
	boost::for_each(blocking::build_blocks(elements, c.get()), [&res](const block & b) { b.as_surfaces(std::back_inserter(res)); });
	return res;
}

} // namespace operations