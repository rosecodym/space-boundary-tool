#pragma once

#include "precompiled.h"

namespace blocking {

namespace impl {

template <typename OutputIterator>
std::set<size_t> scan_for_degenerate_halfblocks(const relations_grid & surface_relationships, size_t face_count, const element & e, OutputIterator oi) {
	std::set<size_t> res;
	for (size_t i = 0; i < face_count; ++i) {
		size_t j;
		for (j = 0; j < face_count; ++j) {
			if (i != j && surface_relationships[i][j].are_parallel()) {
				break;
			}
		}
		if (j == face_count) {
			*oi++ = surface_relationships[i][i].to_halfblock(e);
			res.insert(i);
		}
	}
	return res;
}

} // namespace impl

} // namespace blocking