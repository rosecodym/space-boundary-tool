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
	polygon_with_holes_3(const PointRange & outer, const HoleRange & holes) : m_outer(outer.begin(), outer.end()), m_holes(holes.begin(), holes.end()) { 
		geometry_common::cleanup_loop(&m_outer, g_opts.equality_tolerance);
		boost::for_each(m_holes, [](std::vector<point_3> & hole) {
			geometry_common::cleanup_loop(&hole, g_opts.equality_tolerance);
		});
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

	std::string to_string() const {
		auto loop_to_string = [](const std::vector<point_3> & loop) -> std::string {
			std::stringstream ss;
			boost::for_each(loop, [&ss](const point_3 & p) { ss << "(" << CGAL::to_double(p.x()) << ", " << CGAL::to_double(p.y()) << ", " << CGAL::to_double(p.z()) << ")\n"; });
			return ss.str();
		};
		std::stringstream ss;
		ss << "Outer:\n" << loop_to_string(outer());
		boost::for_each(m_holes, [&ss, &loop_to_string](const std::vector<point_3> & hole) {
			ss << "Hole:\n" << loop_to_string(hole);
		});
		return ss.str();
	}

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