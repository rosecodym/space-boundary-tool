#include "precompiled.h"

#include "equality_context.h"
#include "printing-macros.h"

#include "geometry_2d_common.h"

namespace geometry_2d {

bool cleanup_polygon(polygon_2 * poly, double eps) {
	if (FLAGGED((SBT_VERBOSE_BLOCKS | SBT_VERBOSE_STACKS))) {
		NOTIFY_MSG("Entered geometry_2d::cleanup_polygon.\n");
		PRINT_POLYGON(*poly);
	}

	std::vector<point_2> res;

	std::for_each(poly->vertices_begin(), poly->vertices_end(), [poly, &res, eps](const point_2 & p) {
		if (res.empty()) {
			res.push_back(p);
		}
		else if (p != res.back()) {
			if (res.size() == 1) {
				res.push_back(p);
			}
			else if (!equality_context::are_effectively_collinear(p, res[res.size() - 1], res[res.size() - 2], eps)) {
				res.push_back(p);
			}
			else {
				res.back() = p;
			}
		}
	});

	while (res.size() > 3) {
		if (equality_context::are_effectively_same(res.front(), res.back(), eps)) {
			res.pop_back();
			continue;
		}
		if (equality_context::are_effectively_collinear(res[res.size() - 2], res.back(), res.front(), eps)) {
			res.pop_back();
			continue;
		}
		if (equality_context::are_effectively_collinear(res.back(), res.front(), res[1], eps)) {
			res.front() = res.back();
			continue;
		}
		break;
	}

	if (res.size() < poly->size()) {
		*poly = polygon_2(res.begin(), res.end());
	}
	if (FLAGGED((SBT_VERBOSE_BLOCKS | SBT_VERBOSE_STACKS))) {
		NOTIFY_MSG("Exiting geometry_2d::cleanup_polygon.\n");
		PRINT_POLYGON(*poly);
	}

	return res.size() >= 3;
}

} // namespace geometry_2d