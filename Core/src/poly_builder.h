#pragma once

#include "precompiled.h"

#include "equality_context.h"
#include "printing-macros.h"

namespace solid_geometry {

namespace impl {

class poly_builder : public CGAL::Modifier_base<polyhedron_3::HDS> {
private:
	std::vector<point_3> points;
	std::vector<std::deque<size_t>> indices;
public:
	template <typename FaceRange>
	poly_builder(const FaceRange & faces) {
		std::map<point_3, size_t> point_lookup;
		for (auto f = faces.begin(); f != faces.end(); ++f) {
			indices.push_back(std::deque<size_t>());
			boost::for_each(f->outer(), [&point_lookup, this](const point_3 & p) {
				auto exists = point_lookup.find(p);
				if (exists == point_lookup.end()) {
					exists = point_lookup.insert(std::make_pair(p, points.size())).first;
					points.push_back(p);
				}
				indices.back().push_back(exists->second);
			});
		}
	}

	template <typename PointRange>
	poly_builder(const PointRange & base_loop, const transformation_3 & extrude, equality_context * c) {
		std::deque<size_t> base_indices;
		std::deque<size_t> target_indices;
		boost::transform(base_loop, std::back_inserter(points), [c](const point_3 & p) { return c->snap(p); });
		size_t base_count = points.size();
		boost::transform(base_loop, std::back_inserter(points), [&extrude, c](const point_3 & p) { return c->snap(extrude(p)); });
		if (FLAGGED(SBT_VERBOSE_ELEMENTS)) {
			NOTIFY_MSG("Creating an extrusion polygon from the following points:\n");
			PRINT_LOOP_3(points);
		}
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
		indices.push_back(std::deque<size_t>(target_indices.rbegin(), target_indices.rend()));
	}
	
	void operator () (polyhedron_3::HDS & hds) {
		CGAL::Polyhedron_incremental_builder_3<polyhedron_3::HDS> b(hds, true);
		b.begin_surface(points.size(), indices.size());
		for (auto p = points.begin(); p != points.end(); ++p) {
			b.add_vertex(*p);
		}
		for (auto f = indices.begin(); f != indices.end(); ++f) {
			b.add_facet(f->begin(), f->end());
		}
		b.end_surface();
	}

};

} // namespace impl

} // namespace solid_geometry