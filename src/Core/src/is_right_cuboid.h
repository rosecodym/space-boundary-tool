#pragma once

#include "precompiled.h"

#include "report.h"
#include "surface_pair.h"

class element;

namespace blocking {

namespace impl {

template <typename BlockOutputIterator>
bool is_right_cuboid(
	const relations_grid & surf_rels, 
	size_t face_count, 
	const element & e, 
	double max_distance,
	BlockOutputIterator oi) 
{
	if (face_count == 6) {

		typedef std::pair<boost::optional<size_t>, boost::optional<size_t>> block_entry;
		enum check_result { MATCH, FAILURE, INDETERMINATE };
		auto check_against_block_entry = [=, &surf_rels](block_entry * entry, size_t ix) -> check_result {
			if (!entry->first) { 
				entry->first = ix;
				return MATCH;
			}
			else if (surf_rels[*entry->first][ix].is_orthogonal_translation()) {
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

		auto distance = [&](size_t ix) -> double {
			auto & pair = surf_rels[*blocks[ix].first][*blocks[ix].second];
			NT dist = abs(pair.base().height() - pair.other().height());
			return CGAL::to_double(dist);
		};

		for (size_t i = 0; i < 3; ++i) {
			size_t base_ix = *blocks[i].first;
			size_t other_ix = *blocks[i].second;
			if (distance(i) <= max_distance) {
				*oi++ = surf_rels[base_ix][other_ix].to_block(e);
			}
			else {
				*oi++ = surf_rels[base_ix][other_ix].to_halfblock(e);
				*oi++ = surf_rels[other_ix][base_ix].to_halfblock(e);
			}
		}
		
		reporting::report_progress("element is a right cuboid. ");
		return true;
	}
	return false;
}

} // namespace impl

} // namespace blocking