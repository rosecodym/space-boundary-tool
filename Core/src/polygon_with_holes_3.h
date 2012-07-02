#pragma once

#include "precompiled.h"

#include "polygon_with_holes_2.h"
#include "printing-macros.h"

class polygon_with_holes_3 {
private:
	std::vector<point_3> m_outer;
	std::vector<std::vector<point_3>> m_holes;

	static std::vector<point_3> transform_loop(const std::vector<point_3> & loop, const transformation_3 & t) {
		std::vector<point_3> res;
		boost::transform(loop, std::back_inserter(res), [&t](const point_3 & p) { return t(p); });
		return res;
	}

public:
	template <typename PointRange, typename HoleRange>
	polygon_with_holes_3(const PointRange & outer, const HoleRange & holes) : m_outer(outer.begin(), outer.end()), m_holes(holes.begin(), holes.end()) { 
		if (m_outer.size() < 3) {
			NOTIFY_MSG("Polygon with holes is bad!\n");
			PRINT_LOOP_3(m_outer);
		}
	}

	const std::vector<point_3> & outer() const { return m_outer; }

	polygon_with_holes_3 transform(const transformation_3 & t) const { 
		return polygon_with_holes_3(
			transform_loop(m_outer, t),
			m_holes | boost::adaptors::transformed([&t](const std::vector<point_3> & hole) { return transform_loop(hole, t); }));
	}

	polygon_with_holes_2 project_flat() const {
		return polygon_with_holes_2(
			m_outer | boost::adaptors::transformed([](const point_3 & p) { return point_2(p.x(), p.y()); }),
			m_holes | boost::adaptors::transformed([](const std::vector<point_3> & hole) -> std::vector<point_2> {
				std::vector<point_2> res;
				boost::transform(hole, std::back_inserter(res), [](const point_3 & p) { return point_2(p.x(), p.y()); });
				return res;
			}));
	}

	void print_outer() const { PRINT_LOOP_3(m_outer); }
};