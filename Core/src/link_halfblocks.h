#pragma once

#include "precompiled.h"

class element;

namespace blocking {

namespace impl {

template <typename OutputIterator>
void link_halfblocks(std::list<oriented_area> && halfblocks, const element & e, OutputIterator oi) {
	while (!halfblocks.empty()) {
		std::list<oriented_area>::iterator curr_other = halfblocks.end();
		boost::optional<NT> curr_height;
		for (auto other = halfblocks.begin(); other != halfblocks.end(); ++other) {
			if (other != halfblocks.begin()) {
				boost::optional<NT> this_height = oriented_area::could_form_block(halfblocks.front(), *other);
				if (this_height && (!curr_height || *this_height < *curr_height)) {
					curr_height = this_height;
					curr_other = other;
				}
			}
		}
		if (curr_other != halfblocks.end()) {
			*oi++ = block(halfblocks.front(), *curr_other, e);
			halfblocks.erase(curr_other);
		}
		else {
			*oi++ = block(halfblocks.front(), e);
		}
		halfblocks.pop_front();
	}
}

} // namespace impl

} // namespace blocking