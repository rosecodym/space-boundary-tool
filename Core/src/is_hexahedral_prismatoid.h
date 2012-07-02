#pragma once

#include "precompiled.h"

#include "element.h"
#include "surface_pair.h"

namespace blocking {

namespace impl {

template <typename BlockOutputIterator>
bool is_hexahedral_prismatoid(const relations_grid & surface_relationships, size_t face_count, const element & e, BlockOutputIterator oi) {
	if (face_count == 6) {

		auto find_base_pair = [&surface_relationships, face_count](std::pair<size_t, size_t> * pair) -> bool {
			for (size_t i = 0; i < face_count; ++i) {
				for (size_t j = i + 1; j < face_count; ++j) {
					if (surface_relationships[i][j].is_orthogonal_translation()) {
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

		NOTIFY_MSG("element is a hexahedral prismatoid. ");

		*oi++ = surface_relationships[base_pair.first][base_pair.second].to_block(e);
		handled[base_pair.first] = handled[base_pair.second] = true;

		auto is_in_base_pair = [base_pair](size_t face_ix) { return base_pair.first == face_ix || base_pair.second == face_ix; };
		for (size_t i = 0; i < face_count; ++i) {
			if (!is_in_base_pair(i)) {
				for (size_t j = i + 1; j < face_count; ++j) {
					const surface_pair & rel = surface_relationships[i][j];
					const surface_pair & other = surface_relationships[j][i];
					if (!is_in_base_pair(j) && rel.are_parallel()) {
						boost::optional<oriented_area> a;
						if (!(a = rel.base_minus_other_projected())->area_2d().is_empty()) { *oi++ = block(*a, e); }
						if (!(a = other.base_minus_other_projected())->area_2d().is_empty()) { *oi++ = block(*a, e); }
						if (!(a = rel.base_intr_other_projected())->area_2d().is_empty()) { *oi++ = block(*a, other.base_intr_other_projected(), e); }
						handled[i] = handled[j] = true;
						break;
					}
				}
			}
		}

		for (size_t i = 0; i < 6; ++i) {
			if (!handled[i]) {
				*oi++ = surface_relationships[i][i].to_halfblock(e);
			}
		}

		return true;
	}
	return false;
}

} // namespace impl

} // namespace blocking