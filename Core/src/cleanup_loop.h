#pragma once

#include "precompiled.h"

#include "equality_context.h"

namespace geometry_common {

namespace impl {

template <typename Loop>
std::list<typename Loop::value_type> create_cleaned_loop(const Loop & loop, double eps) {
	typedef typename Loop::value_type point_t;
	typedef typename std::list<point_t>::iterator iter_t;

	if (boost::distance(loop) < 3) { return std::list<point_t>(); }

	std::list<point_t> points(loop.begin(), loop.end());
	iter_t window[3];

	auto collapse = [&points, eps](iter_t w[3]) -> bool {
		if (equality_context::are_effectively_same(*w[1], *w[2], eps)) {
			points.erase(w[2]);
			return true;
		}
		else if (equality_context::are_effectively_same(*w[0], *w[2], eps)) {
			points.erase(w[1]);
			points.erase(w[2]);
			return true;
		}
		else if (
			equality_context::are_effectively_same(*w[0], *w[1], eps) ||
			equality_context::are_effectively_collinear(*w[0], *w[1], *w[2], eps))
		{
			points.erase(w[1]);
			return true;
		}
		return false;
	};

	auto next = [&points](iter_t iter) {
		return ++iter == points.end() ? points.begin() : iter;
	};

	window[0] = points.begin();
	int steps_since_last_change = 0;
	while (points.size() >= 3 && steps_since_last_change < points.size()) {
		window[1] = next(window[0]);
		window[2] = next(window[1]);
		if (!collapse(window)) {
			window[0] = next(window[0]);
			++steps_since_last_change;
		}
		else { steps_since_last_change = 0; }
	}

	return points.size() >= 3 ? points : std::list<point_t>();
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