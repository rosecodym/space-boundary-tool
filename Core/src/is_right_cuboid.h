#pragma once

#include "precompiled.h"

#include "report.h"
#include "surface_pair.h"

class element;

namespace blocking {

namespace impl {

template <typename BlockOutputIterator>
bool is_right_cuboid(const relations_grid & surface_relationships, size_t face_count, const element & e, BlockOutputIterator oi) {
	if (face_count == 6) {

		typedef std::pair<boost::optional<size_t>, boost::optional<size_t>> block_entry;
		enum check_result { MATCH, FAILURE, INDETERMINATE };
		auto check_against_block_entry = [=, &surface_relationships](block_entry * entry, size_t ix) -> check_result {
			if (!entry->first) { 
				entry->first = ix;
				return MATCH;
			}
			else if (surface_relationships[*entry->first][ix].is_orthogonal_translation()) {
				if (!entry->second) {
					entry->second = ix;
					return MATCH;
				}
				else {
					return FAILURE;
				}
			}
			else {
				return INDETERMINATE;
			}
		};

		block_entry blocks[3];

		for (size_t i = 0; i < 6; ++i) {
			check_result res = INDETERMINATE;
			for (size_t b_ix = 0; b_ix < 3; ++b_ix) {
				res = check_against_block_entry(&blocks[b_ix], i);
				if (res != INDETERMINATE) { break; }
			}
			if (res != MATCH) { return false; }
		}

		*oi++ = surface_relationships[*blocks[0].first][*blocks[0].second].to_block(e);
		*oi++ = surface_relationships[*blocks[1].first][*blocks[1].second].to_block(e);
		*oi++ = surface_relationships[*blocks[2].first][*blocks[2].second].to_block(e);
		
		reporting::report_progress("element is a right cuboid. ");
		return true;
	}
	return false;
}

} // namespace impl

} // namespace blocking