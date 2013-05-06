#include "precompiled.h"

#include "poly_builder.h"

#include "stringification.h"

namespace solid_geometry {

namespace impl {

void poly_builder::operator () (polyhedron_3::HDS & hds) {
	assert(all_faces_planar());
	CGAL::Polyhedron_incremental_builder_3<polyhedron_3::HDS> b(hds, true);
	b.begin_surface(points_.size(), indices_.size());
	for (auto p = points_.begin(); p != points_.end(); ++p) {
		b.add_vertex(*p);
	}
	for (auto f = indices_.begin(); f != indices_.end(); ++f) {
		b.add_facet(f->begin(), f->end());
	}
	b.end_surface();
}

bool poly_builder::all_faces_planar() const {
	for (auto p = indices_.begin(); p != indices_.end(); ++p) {
		auto & vertices = *p;
		plane_3 pl(
			points_[vertices[0]], 
			points_[vertices[1]], 
			points_[vertices[2]]);
		for (size_t i = 3; i < vertices.size(); ++i) {
			if (!pl.has_on(points_[vertices[i]])) { return false; }
		}
	}
	return true;
}

std::string poly_builder::to_string() const {
	std::stringstream ss;
	ss << "Poly builder for:\n" << reporting::to_string(points_);
	for (size_t i = 0; i < indices_.size(); ++i) {
		ss << "Facet " << i << ":\n";
		for (auto p = indices_[i].begin(); p != indices_[i].end(); ++p) {
			auto pt_string = reporting::to_string(points_[*p]);
			ss << (boost::format("  [%u]\t%s\n") % *p % pt_string).str();
		}
	}
	return ss.str();
}

} // namespace impl

} // namespace solid_geometry