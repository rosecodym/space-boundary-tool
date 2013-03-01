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

		std::pair<size_t, size_t> base_pair;
		if (!find_base_pair(&base_pair)) {
			return false;
		}

		reporting::report_progress("element is a hexahedral prismatoid. ");

		auto & bpair = surf_rels[base_pair.first][base_pair.second];
		NT dist = abs(
			CGAL::to_double(bpair.base().height()) - 
			CGAL::to_double(bpair.other().height()));
		if (CGAL::to_double(dist) <= max_distance) {
			*oi++ = bpair.to_block(e);
		}
		else {
			*oi++ = bpair.to_halfblock(e);
			*oi++ = bpair.opposite().to_halfblock(e);
		}
		handled[base_pair.first] = handled[base_pair.second] = true;

		auto is_in_base_pair = [base_pair](size_t face_ix) { return base_pair.first == face_ix || base_pair.second == face_ix; };
		for (size_t i = 0; i < face_count; ++i) {
			if (!is_in_base_pair(i)) {
				for (size_t j = i + 1; j < face_count; ++j) {
					const surface_pair & rel = surf_rels[i][j];
					const surface_pair & other = surf_rels[j][i];
					if (!is_in_base_pair(j) && rel.are_parallel()) {
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
							area oth_intr = other.base_intr_other_projected();
							oriented_area rel_part = 
								rel.base_part_with_area(a);
							oriented_area other_part = 
								other.base_part_with_area(oth_intr);
							*oi++ = block(rel_part, other_part, e);
						}
						handled[i] = handled[j] = true;
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