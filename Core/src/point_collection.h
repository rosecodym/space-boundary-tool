#pragma once

#include "precompiled.h"

#include <CGAL/Kd_tree.h>
#include <CGAL/Search_traits_3.h>
#include <CGAL/Fuzzy_sphere.h>

class point_collection {
private:
	typedef CGAL::Search_traits_3<K>			search_traits;
	typedef CGAL::Kd_tree<search_traits>		kd_tree;
	typedef CGAL::Fuzzy_sphere<search_traits>	fuzzy_sphere;

	std::unique_ptr<kd_tree> tree; // WHY IS kd_tree::search NOT CONST WHAT THE HELL
	double epsilon;

	point_collection();
	point_collection(const point_collection & src);
	point_collection & operator = (const point_collection & src);
public:
	template <class PointRange> point_collection(PointRange points, double eps) : tree(new kd_tree(points.begin(), points.end())), epsilon(eps) { }
	point_collection(point_collection && src) : tree(std::move(src.tree)), epsilon(src.epsilon) { }
	point_collection & operator = (point_collection && src) { tree = std::move(src.tree); epsilon = src.epsilon; }

	point_3 get_near(const point_3 & p) const { 
		std::vector<point_3> results;
		tree->search(std::back_inserter(results), fuzzy_sphere(p, epsilon));
		return results.front();
	}
};