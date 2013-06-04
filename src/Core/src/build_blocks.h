#pragma once

#include "precompiled.h"

#include "element.h"
#include "block.h"

class equality_context;
class surface;

namespace blocking {

namespace impl {

// A negative max_block_thickness signifies "infinite."
std::vector<block> build_blocks_for(
	const element & e,
	equality_context * c, 
	double max_block_thickness = -1);

} // namespace impl

// A negative max_block_thickness signifies "infinite."
std::vector<block> build_blocks(
	const std::vector<element> & elements,
	equality_context * c, 
	double max_block_thickness = -1);

} // namespace blocking