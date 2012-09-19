#pragma once

#include "precompiled.h"

class polygon_with_holes_2 {
private:
	polygon_2 m_outer;
	std::vector<polygon_2> m_holes;

	void cleanup();

public:
	polygon_with_holes_2() { }

	template <typename HoleRange>
	polygon_with_holes_2(const polygon_2 & outer, const HoleRange & holes) : m_outer(outer), m_holes(holes.begin(), holes.end()) { 
		cleanup();
	}

	template <typename PointRange, typename HoleRange>
	polygon_with_holes_2(const PointRange & outer, const HoleRange & holes) : m_outer(outer.begin(), outer.end()) {
		boost::transform(holes, std::back_inserter(m_holes), [](const std::vector<point_2> & hole) {
			return polygon_2(hole.begin(), hole.end());
		});
		cleanup();
	}

	const polygon_2 & outer() const { return m_outer; }
	const std::vector<polygon_2> & holes() const { return m_holes; }

	void reverse();

	bool is_axis_aligned() const;

	std::vector<polygon_2> all_polygons() const;
};