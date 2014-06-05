#pragma once

#include "precompiled.h"

#include "cleanup_loop.h"
#include "geometry_common.h"
#include "polygon_with_holes_2.h"
#include "report.h"
#include "sbt-core.h"

extern sb_calculation_options g_opts;

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
	polygon_with_holes_3(const PointRange & outer, const HoleRange & holes) 
		: m_outer(outer.begin(), 
		outer.end()), 
		m_holes(holes.begin(), holes.end()) 
	{ 
		geometry_common::cleanup_loop(&m_outer, g_opts.tolernace_in_meters);
		boost::for_each(m_holes, [](std::vector<point_3> & hole) {
			geometry_common::cleanup_loop(&hole, g_opts.tolernace_in_meters);
		});
	}

	const std::vector<point_3> & outer() const { return m_outer; }

	polygon_with_holes_3 transform(const transformation_3 & t) const { 
		std::vector<std::vector<point_3>> new_holes;
		for (auto h = m_holes.begin(); h != m_holes.end(); ++h) {
			new_holes.push_back(transform_loop(*h, t));
		}
		return polygon_with_holes_3(transform_loop(m_outer, t), new_holes);
	}

	polygon_with_holes_2 project_flat() const;

	std::string to_string() const;

	friend bool operator == (const polygon_with_holes_3 & a, const polygon_with_holes_3 & b);
};

inline bool operator == (const polygon_with_holes_3 & a, const polygon_with_holes_3 & b) {
	auto loops_equal = [](const std::vector<point_3> & a, const std::vector<point_3> & b) -> bool {
		if (a.size() != b.size()) { return false; }
		auto b_match = b.begin();
		for (; b_match != b.end(); ++b_match) { if (*b_match == a.front()) { break; } }
		if (b_match == b.end()) { return false; }
		auto next_b = [&b_match, &b]() -> point_3 {
			if (b_match == b.end()) { b_match = b.begin(); }
			return *b_match++;
		};
		for (auto p = a.begin(); p != a.end(); ++p) {
			if (*p != next_b()) { return false; }
		}
		return true;
	};

	if (a.m_holes.size() != b.m_holes.size()) { return false; }
	if (!loops_equal(a.outer(), b.outer())) { return false; }
	for (size_t i = 0; i < a.m_holes.size(); ++i) {
		if (!loops_equal(a.m_holes[i], b.m_holes[i])) { return false; }
	}
	return true;
}