#pragma once

#include "precompiled.h"

#include "equality_context.h"

namespace solid_geometry {

namespace impl {

class poly_builder : public CGAL::Modifier_base<polyhedron_3::HDS> {
private:
	std::vector<point_3> points_;
	std::vector<std::deque<size_t>> indices_;

	poly_builder(
		const std::vector<point_3> & points,
		const std::vector<std::deque<size_t>> & indices)
		: points_(points),
		  indices_(indices)
	{ }

public:
	void operator () (polyhedron_3::HDS & hds);

	bool all_faces_planar() const;

	std::string to_string() const;
	
	template <typename FaceRange>
	static poly_builder create(const FaceRange & faces) {
		std::vector<point_3> points;
		std::vector<std::deque<size_t>> indices;
		std::map<point_3, size_t> point_lookup;

		for (auto f = faces.begin(); f != faces.end(); ++f) {
			indices.push_back(std::deque<size_t>());
			assert(f->inners().size() == 0);
			for (auto p = f->outer().begin(); p != f->outer().end(); ++p) {
				auto exists = point_lookup.find(*p);
				if (exists == point_lookup.end()) {
					auto new_entry = std::make_pair(*p, points.size());
					exists = point_lookup.insert(new_entry).first;
					points.push_back(*p);
				}
				indices.back().push_back(exists->second);
			}
		}
		return poly_builder(points, indices);
	}

	template <typename PointRange>
	static poly_builder create(
		const PointRange & base_loop, 
		const transformation_3 & extrude)
	{
		std::vector<point_3> points;
		std::vector<std::deque<size_t>> indices;
		std::deque<size_t> base_indices;
		std::deque<size_t> target_indices;

		boost::copy(base_loop, std::back_inserter(points));
		size_t base_count = points.size();
		boost::transform(base_loop, std::back_inserter(points), extrude);
		for (size_t i = 0; i < base_count; ++i) {
			base_indices.push_back(i);
			target_indices.push_back(i + base_count);
		}
		indices.resize(base_count);
		for (size_t i = 0; i < base_count; ++i) {
			indices[i].push_back(target_indices[i]);
			indices[i].push_back(base_indices[i]);
			indices[(i + 1) % base_count].push_front(target_indices[i]);
			indices[(i + 1) % base_count].push_front(base_indices[i]);
		}
		indices.push_back(base_indices);
		indices.push_back(std::deque<size_t>(
			target_indices.rbegin(), 
			target_indices.rend()));
		return poly_builder(points, indices);
	}
};

} // namespace impl

} // namespace solid_geometry