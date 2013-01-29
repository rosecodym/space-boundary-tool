#include "precompiled.h"

#include "polygon_with_holes_3.h"

polygon_with_holes_2 polygon_with_holes_3::project_flat() const {
	std::vector<point_2> outer;
	std::vector<std::vector<point_2>> holes;
	for (auto p = m_outer.begin(); p != m_outer.end(); ++p) {
		outer.push_back(point_2(p->x(), p->y()));
	}
	for (auto h = m_holes.begin(); h != m_holes.end(); ++h) {
		holes.push_back(std::vector<point_2>());
		for (auto p = h->begin(); p != h->end(); ++p) {
			holes.back().push_back(point_2(p->x(), p->y()));
		}
	}
	return polygon_with_holes_2(outer, holes);
}

std::string polygon_with_holes_3::to_string() const {
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