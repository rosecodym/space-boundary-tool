#pragma once

#include "precompiled.h"

#include "equality_context.h"

#include "report.h"
#include "stringification.h"

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
	poly_builder(const PointRange & base_loop, const transformation_3 & extrude) {
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
		indices.push_back(std::deque<size_t>(target_indices.rbegin(), target_indices.rend()));
	}
	
	void operator () (polyhedron_3::HDS & hds) {
		assert(all_faces_planar());
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

	bool all_faces_planar() const {
		for (auto p = indices.begin(); p != indices.end(); ++p) {
			auto & vertices = *p;
			plane_3 pl(
					points[vertices[0]], 
					points[vertices[1]], 
					points[vertices[2]]);
			for (size_t i = 3; i < vertices.size(); ++i) {
				if (!pl.has_on(points[vertices[i]])) { return false; }
			}
		}
		return true;
	}

	std::string to_string() const {
		std::stringstream ss;
		ss << "Poly builder for:\n" << reporting::to_string(points);
		for (size_t i = 0; i < indices.size(); ++i) {
			ss << "Facet " << i << ":\n";
			for (auto p = indices[i].begin(); p != indices[i].end(); ++p) {
				auto pt_string = reporting::to_string(points[*p]);
				ss << (boost::format("  [%u]\t%s\n") % *p % pt_string).str();
			}
		}
		return ss.str();
	}
};

} // namespace impl

} // namespace solid_geometry