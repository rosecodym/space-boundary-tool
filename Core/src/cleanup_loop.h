#pragma once

#include "precompiled.h"

#include "equality_context.h"

namespace geometry_common {

namespace impl {

template <typename Loop>
std::deque<typename Loop::value_type> create_cleaned_loop(const Loop & loop, double eps) {
	typedef typename Loop::value_type point_t;
	std::deque<point_t> res;
	boost::for_each(loop, [&res, eps](const point_t & p) {
		if (res.empty()) {
			res.push_back(p);
		}
		else if (!equality_context::are_effectively_same(p, res.back(), eps)) {
			if (res.size() == 1 || !equality_context::are_effectively_collinear(p, res.back(), res[res.size() - 2], eps)) {
				res.push_back(p);
			}
			else {
				res.back() = p;
			}
		}
	});
	while (res.size() >= 3) {
		if (equality_context::are_effectively_same(res.front(), res.back(), eps)) {
			res.pop_back();
			continue;
		}
		if (equality_context::are_effectively_collinear(res[res.size() - 2], res.back(), res.front(), eps)) {
			res.pop_back();
			continue;
		}
		if (equality_context::are_effectively_collinear(res.back(), res[0], res[1], eps)) {
			res.pop_front();
			continue;
		}
		break;
	}
	return res;
}

} // namespace impl

template <typename Loop>
bool cleanup_loop(Loop * loop, double eps) {
	auto cleaned = impl::create_cleaned_loop(*loop, eps);
	if (cleaned.size() >= 3) {
		*loop = Loop(cleaned.begin(), cleaned.end());
		return true;
	}
	else {
		return false;
	}
}

inline bool cleanup_loop(polygon_2 * poly, double eps) {
	auto cleaned = impl::create_cleaned_loop(std::vector<point_2>(poly->vertices_begin(), poly->vertices_end()), eps);
	if (cleaned.size() >= 3) {
		*poly = polygon_2(cleaned.begin(), cleaned.end());
		return true;
	}
	else {
		return false;
	}
}

} // namespace geometry_common