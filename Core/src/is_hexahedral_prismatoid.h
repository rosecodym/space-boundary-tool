#pragma once

#include "precompiled.h"

#include "report.h"
#include "surface_pair.h"

class element;

namespace blocking {

namespace impl {

template <typename BlockOutputIterator>
bool is_hexahedral_prismatoid(
	const relations_grid & surf_rels, 
	size_t face_count, 
	const element & e,
	double max_distance, 
	BlockOutputIterator oi) 
{
	if (face_count == 6) {

		auto find_base_pair = [&surf_rels, face_count](std::pair<size_t, size_t> * pair) -> bool {
			for (size_t i = 0; i < face_count; ++i) {
				for (size_t j = i + 1; j < face_count; ++j) {
					if (surf_rels[i][j].is_orthogonal_translation()) {
						pair->first = i;
						pair->second = j;
						return true;
					}
				}
			}
			return false;
		};

		std::vector<bool> handled(6, false);

		std::pair<size_t, size_t> bpair;
		if (!find_base_pair(&bpair)) {
			return false;
		}

		reporting::report_progress("element is a hexahedral prismatoid. ");

		auto dist = [&surf_rels](size_t i, size_t j) -> double {
			double a = CGAL::to_double(surf_rels[i][i].base().height());
			double b = CGAL::to_double(surf_rels[j][j].base().height());
			return abs(a - b);
		};

		if (dist(bpair.first, bpair.second) <= max_distance) {
			*oi++ = surf_rels[bpair.first][bpair.second].to_block(e);
		}
		else {
			*oi++ = surf_rels[bpair.first][bpair.second].to_halfblock(e);
			*oi++ = surf_rels[bpair.second][bpair.first].to_halfblock(e);
		}
		handled[bpair.first] = handled[bpair.second] = true;

		auto is_in_base_pair = [bpair](size_t face_ix) { 
			return bpair.first == face_ix || bpair.second == face_ix; 
		};
		for (size_t i = 0; i < face_count; ++i) {
			if (!is_in_base_pair(i)) {
				for (size_t j = i + 1; j < face_count; ++j) {
					const surface_pair & rel = surf_rels[i][j];
					const surface_pair & other = surf_rels[j][i];
					if (!is_in_base_pair(j) && rel.are_parallel()) {
						if (dist(i, j) <= max_distance) {
							area a;
							a = rel.base_minus_other_projected();
							if (!a.is_empty()) {
								*oi++ = block(rel.base_part_with_area(a), e);
							}
							a = other.base_minus_other_projected();
							if (!a.is_empty()) {
								*oi++ = block(other.base_part_with_area(a), e);
							}
							a = rel.base_intr_other_projected();
							if (!a.is_empty()) {
								area oth_intr = 
									other.base_intr_other_projected();
								oriented_area rel_part = 
									rel.base_part_with_area(a);
								oriented_area other_part = 
									other.base_part_with_area(oth_intr);
								*oi++ = block(rel_part, other_part, e);
							}
							handled[i] = handled[j] = true;
						}
						break;
					}
				}
			}
		}

		for (size_t i = 0; i < 6; ++i) {
			if (!handled[i]) {
				*oi++ = surf_rels[i][i].to_halfblock(e);
			}
		}

		return true;
	}
	return false;
}

} // namespace impl

} // namespace blocking