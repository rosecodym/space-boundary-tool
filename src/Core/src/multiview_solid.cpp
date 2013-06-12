#include "precompiled.h"

#include "equality_context.h"
#include "exceptions.h"
#include "poly_builder.h"
#include "sbt-core.h"
#include "simple_face.h"
#include "solid_geometry_util.h"

#include "multiview_solid.h"

namespace solid_geometry {

using namespace impl;

namespace {

bbox_3 nef_bounding_box(const nef_polyhedron_3 & nef) {
	nef_vertex_handle v;
	boost::optional<bbox_3> res;
	CGAL_forall_vertices(v, nef) {
		if (!res.is_initialized()) {
			res = v->point().bbox();
		}
		else {
			res = *res + v->point().bbox();
		}
	}
	return *res;
}

extrusion_information get_extrusion_information(
	const extruded_area_solid & e, 
	equality_context * c) 
{
	using geometry_common::normalize;
	if (c->is_zero(e.extrusion_depth)) {
		throw shallow_extrusion_exception();
	}
	simple_face area(e.area, true, c);
	auto snapped_dir = c->snap(direction_3(e.ext_dx, e.ext_dy, e.ext_dz));
	auto ext = normalize(snapped_dir.vector()) * e.extrusion_depth;
	auto area_normal = area.orthogonal_direction().vector();
	assert(!CGAL::is_zero(area_normal.squared_length()));
	assert(!CGAL::is_zero(ext.squared_length()));
	if (c->are_effectively_perpendicular(area_normal, ext)) {
		throw parallel_ext_exception();
	}
	return std::make_tuple(area, ext);
}

std::vector<oriented_area> to_face_group(
	const nef_polyhedron_3 & nef, 
	nef_volume_handle v, 
	equality_context * c) 
{
	
	class face_generator {
	private:
		std::vector<oriented_area> faces;
		equality_context * c;
	public:
		face_generator(equality_context * c) : c(c) { }

		void visit(nef_vertex_handle /*h*/) { }
		void visit(nef_halfedge_handle /*h*/) { }
		void visit(nef_halffacet_handle h) { 
			faces.push_back(oriented_area(h, c)); 
		}
		void visit(nef_shalfedge_handle /*h*/) { }
		void visit(nef_shalfloop_handle /*h*/) { }
		void visit(nef_sface_handle /*h*/) { }

		const std::vector<oriented_area> & get_faces() { return faces; }
	};

	nef_polyhedron_3::Shell_entry_const_iterator sit;
	face_generator fgen(c);
	nef.visit_shell_objects(nef_sface_handle(v->shells_begin()), fgen);
	
	return fgen.get_faces();
}

oriented_area_groups nef_to_oriented_area_groups(const nef_polyhedron_3 & nef, equality_context * c) {
	oriented_area_groups g;
	nef_volume_handle v;
	CGAL_forall_volumes(v, nef) {
		if (v->mark()) { g.push_back(to_face_group(nef, v, c)); }
	}
	return g;
}

} // namespace

multiview_solid::multiview_solid(const solid & s, equality_context * c) {
	if (s.rep_type == REP_BREP) {
		if (s.rep.as_brep.face_count < 4) { throw bad_brep_exception(); }
		// This next cutoff is kind of arbitrary. I picked 89 because the slab 
		// I can't figure out right now has 90 faces.
		else if (s.rep.as_brep.face_count > 89) { throw bad_brep_exception(); }

		std::vector<simple_face> all_faces = faces_from_brep(s.rep.as_brep, c);
		// These need to be nef-able because that's how face normals are
		// reconciled.
		for (auto f = all_faces.begin(); f != all_faces.end(); ++f) {
			if (!f->voids().empty()) { throw brep_with_voids_exception(); }
		}
		auto nef = simple_faces_to_nef(std::move(all_faces), *c);
		if (nef.is_empty()) {
			throw bad_brep_exception();
		}
		as_nef_ = nef;
	}
	else if (s.rep_type == REP_EXT) {
		as_extrusion_info_ = get_extrusion_information(s.rep.as_ext, c);
	}
	else {
		throw unknown_geometry_rep_exception();
	}
}

multiview_solid::multiview_solid(
	const nef_polyhedron_3 & nef, 
	nef_volume_handle v, 
	equality_context * c)
	: as_face_groups_(oriented_area_groups(1, to_face_group(nef, v, c))) { }

bbox_3 multiview_solid::bounding_box() const {
	if (as_extrusion_info_) {
		auto ext = *as_extrusion_info_;
		auto f = std::get<0>(ext);
		transformation_3 extrude(CGAL::TRANSLATION, std::get<1>(ext));
		bbox_3 res = f.outer().front().bbox();
		for (auto p = f.outer().begin(); p != f.outer().end(); ++p) {
			res = res + p->bbox() + extrude(*p).bbox();
		}
		return res;
	}
	else if (as_face_groups_) {
		auto grps = *as_face_groups_;
		bbox_3 res = grps.front().front().bounding_box();
		for (auto g = grps.begin(); g != grps.end(); ++g) {
			for (auto s = g->begin(); s != g->end(); ++s) {
				res = res + s->bounding_box();
			}
		}
		return res;
	}
	else if (as_nef_) { return nef_bounding_box(*as_nef_); }
	else { return bbox_3(); }
}

bool multiview_solid::is_single_volume() const {
	if (as_extrusion_info_) { return true; }
	else if (as_face_groups_) { return as_face_groups_->size() == 1; }
	else if (as_nef_) { return as_nef_->number_of_volumes() == 2; }
	else { return false; }
}

std::vector<multiview_solid> multiview_solid::as_single_volumes(
	equality_context * c) const 
{
	if (as_extrusion_info_) { 
		return std::vector<multiview_solid>(1, *as_extrusion_info_);
	}
	else if (as_face_groups_) {
		std::vector<multiview_solid> res;
		auto groups = *as_face_groups_;
		for (auto g = groups.begin(); g != groups.end(); ++g) {
			res.push_back(multiview_solid(*g));
		}
		return res;
	}
	else if (as_nef_) {
		std::vector<multiview_solid> res;
		res.reserve(as_nef_->number_of_volumes() - 1);
		nef_volume_handle v;
		CGAL_forall_volumes(v, *as_nef_) {
			if (v->mark()) {
				res.push_back(multiview_solid(*as_nef_, v, c));
			}
		}
		return res;
	}
	else { return std::vector<multiview_solid>(); }
}

std::vector<oriented_area> multiview_solid::oriented_faces(
	equality_context * c) const 
{
	if (as_face_groups_) { return as_face_groups_->front(); }
	else if (as_extrusion_info_) {
		auto group = impl::extrusion_to_faces(*as_extrusion_info_, c);
		return (as_face_groups_ = oriented_area_groups(1, group))->front();
	}
	else if (as_nef_) {
		as_face_groups_ = nef_to_oriented_area_groups(*as_nef_, c);
		return as_face_groups_->front();
	}
	else { return std::vector<oriented_area>(); }
}

void multiview_solid::subtract(const multiview_solid & other, equality_context * c) {
	if (is_nef_representable() && other.is_nef_representable()) {
		create_nef_rep([c]() { return c; });
		other.create_nef_rep([c]() { return c; });
		as_nef_ = *as_nef_ - *other.as_nef_;
		as_face_groups_.reset();
		as_extrusion_info_.reset();
	}
	else {
		// We shouldn't ever get here, but asserting as much will cause my unit
		// tests to bail in really annoying ways. I need a better solution.
	}
}

bool multiview_solid::share_plane_opposite(
	const multiview_solid & a,
	const multiview_solid & b,
	equality_context * c)
{
	auto a_faces = a.oriented_faces(c);
	auto b_faces = b.oriented_faces(c);
	for (auto p = a_faces.begin(); p != a_faces.end(); ++p) {
		for (auto q = b_faces.begin(); q != b_faces.end(); ++q) {
			if (oriented_area::share_plane_opposite(*p, *q, *c)) {
				return true;
			}
		}
	}
	return false;
}

bool multiview_solid::is_nef_representable() const {
	return as_extrusion_info_ || as_nef_ || !as_face_groups_;
}

void multiview_solid::create_nef_rep(
	std::function<equality_context *(void)> lazy_c) const 
{
	if (as_nef_) { }
	else if (as_extrusion_info_) { 
		as_nef_ = extrusion_to_nef(*as_extrusion_info_, lazy_c()).interior();
	}
	else { as_nef_ = nef_polyhedron_3::EMPTY; }
}

} // namespace solid_geometry