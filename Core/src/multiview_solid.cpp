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

std::vector<simple_face> faces_from_brep(const brep & b, equality_context * c) {
	std::vector<simple_face> res;
	for (size_t i = 0; i < b.face_count; ++i) {
		try {
			res.push_back(simple_face(b.faces[i], c));
		}
		catch (internal_exceptions::invalid_face_exception & /*ex*/) { }
	}
	return res;
}

extrusion_information get_extrusion_information(const extruded_area_solid & e, equality_context * c) {
	return std::make_tuple(simple_face(e.area, c), geometry_common::normalize(c->snap(direction_3(e.ext_dx, e.ext_dy, e.ext_dz)).to_vector()) * e.extrusion_depth);
}

std::vector<oriented_area> to_oriented_face_group(const nef_polyhedron_3 & nef, nef_volume_handle v, equality_context * c) {
	
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
		if (v->mark()) {
			g.push_back(to_oriented_face_group(nef, v, c));
		}
	}
	return g;
}

} // namespace

multiview_solid::multiview_solid(const solid & s, equality_context * c) : m_faces_dropped_during_construction(false) {
	if (s.rep_type == REP_BREP) {
		std::vector<simple_face> all_faces = faces_from_brep(s.rep.as_brep, c);
		if (boost::find_if(all_faces, [](const simple_face & f) { return !f.inners().empty(); }) != all_faces.end()) {
			// if any faces have voids we can't build a nef polyhedron
			// if we can't build a nef polyhedron then we can't fix normals
			// since we don't even have a way to *verify* normals, this is fatal
			throw brep_with_voids_exception();
		}
		if (all_faces.size() != s.rep.as_brep.face_count) { m_faces_dropped_during_construction = true; }
		geometry = simple_faces_to_nef(std::move(all_faces));
	}
	else if (s.rep_type == REP_EXT) {
		geometry = get_extrusion_information(s.rep.as_ext, c);
	}
}

multiview_solid::multiview_solid(const nef_polyhedron_3 & nef, nef_volume_handle v, equality_context * c) {
	geometry = oriented_area_groups();
	boost::get<oriented_area_groups>(geometry).push_back(to_oriented_face_group(nef, v, c));
}

multiview_solid & multiview_solid::operator = (multiview_solid && src) {
	if (&src != this) { 
		geometry = std::move(src.geometry); 
	} 
	return *this; 
}

bbox_3 multiview_solid::bounding_box() const {
	struct bounding_box_visitor : public boost::static_visitor<bbox_3> {
		bbox_3 operator () (const simple_face_groups & simple_faces) const {
			bbox_3 res = simple_faces.front().front().outer().front().bbox();
			for (auto group = simple_faces.begin(); group != simple_faces.end(); ++group) {
				for (auto face = group->begin(); face != group->end(); ++face) {
					for (auto p = face->outer().begin(); p != face->outer().end(); ++p) {
						res = res + p->bbox();
					}
				}
			}
			return res;
		}
		bbox_3 operator () (const oriented_area_groups & oriented_areas) const {
			bbox_3 res = oriented_areas.front().front().bounding_box();
			for (auto group = oriented_areas.begin(); group != oriented_areas.end(); ++group) {
				for (auto s = group->begin(); s != group->end(); ++s) {
					res = res + s->bounding_box();
				}
			}
			return res;
		}
		bbox_3 operator () (const extrusion_information & ext) const {
			const simple_face & f = std::get<0>(ext);
			transformation_3 extrude = transformation_3(CGAL::TRANSLATION, std::get<1>(ext));
			bbox_3 res = f.outer().front().bbox();
			for (auto p = f.outer().begin(); p != f.outer().end(); ++p) {
				res = res + p->bbox() + extrude(*p).bbox();
			}
			return res;
		}
		bbox_3 operator () (const nef_polyhedron_3 & nef) const {
			return nef_bounding_box(nef);
		}
	};
	return boost::apply_visitor(bounding_box_visitor(), geometry);
}

bool multiview_solid::is_single_volume() const {
	struct visitor : public boost::static_visitor<bool> {
		bool operator () (const simple_face_groups & simple_faces) const {
			return simple_faces.size() == 1;
		}
		bool operator () (const oriented_area_groups & oriented_areas) const {
			return oriented_areas.size() == 1;
		}
		bool operator () (const extrusion_information & /*extrusion_information*/) const {
			return true;
		}
		bool operator () (const nef_polyhedron_3 & nef) const {
			return nef.number_of_volumes() == 2;
		}
	};
	return boost::apply_visitor(visitor(), geometry);
}

std::vector<multiview_solid> multiview_solid::as_single_volumes(equality_context * c) const {
	struct visitor : public boost::static_visitor<std::vector<multiview_solid>> {
		equality_context * c;
		visitor(equality_context * c) : c(c) { }
		std::vector<multiview_solid> operator () (const oriented_area_groups & oriented_areas) const {
			std::vector<multiview_solid> res;
			boost::transform(oriented_areas, std::back_inserter(res), [](const std::vector<oriented_area> & group) {
				return multiview_solid(group);
			});
			return res;
		}
		std::vector<multiview_solid> operator () (const extrusion_information & ext) const {
			return std::vector<multiview_solid>(1, ext);
		}
		std::vector<multiview_solid> operator () (const nef_polyhedron_3 & nef) const {
			std::vector<multiview_solid> res;
			res.reserve(nef.number_of_volumes() - 1);
			nef_volume_handle v;
			CGAL_forall_volumes(v, nef) {
				if (v->mark()) {
					res.push_back(multiview_solid(nef, v, c));
				}
			}
			return res;
		}
	};
	return boost::apply_visitor(visitor(c), geometry);
}

std::vector<oriented_area> multiview_solid::oriented_faces(equality_context * c) const {
	struct convert_to_oriented_areas : public boost::static_visitor<geometry_representation> {
		equality_context * c;
		convert_to_oriented_areas(equality_context * c) : c(c) { }
		geometry_representation operator () (const oriented_area_groups & oriented_areas) const { 
			return oriented_areas;
		}
		geometry_representation operator () (const extrusion_information & ext) const {
			return nef_to_oriented_area_groups(impl::extrusion_to_nef(ext, c), c);
		}
		geometry_representation operator () (const nef_polyhedron_3 & nef) const {
			return nef_to_oriented_area_groups(nef, c);
		}
	};
	geometry = boost::apply_visitor(convert_to_oriented_areas(c), geometry);
	oriented_area_groups groups = boost::get<oriented_area_groups>(geometry);
	return groups.front();
}

void multiview_solid::subtract(const multiview_solid & other, equality_context * c) {
	if (is_nef_representable() && other.is_nef_representable()) {
		convert_to_nef([c]() { return c; });
		other.convert_to_nef([c]() { return c; });
		boost::get<nef_polyhedron_3>(geometry) -= boost::get<nef_polyhedron_3>(other.geometry);
	}
}

bool multiview_solid::is_nef_representable() const {
	struct visitor : public boost::static_visitor<bool> {
		bool operator () (const oriented_area_groups & /*oriented_areas*/) const { return false; } // yeah, maybe, but it shouldn't happen and it's hard
		bool operator () (const extrusion_information & /*extrusion_information*/) const { return true; }
		bool operator () (const nef_polyhedron_3 & /*nef*/) const { return true; }
	};
	return boost::apply_visitor(visitor(), geometry);
}

void multiview_solid::convert_to_nef(std::function<equality_context *(void)> lazy_c) const {
	struct visitor : public boost::static_visitor<geometry_representation> {
		std::function<equality_context *(void)> lazy_c;
		visitor(std::function<equality_context *(void)> lazy_c) : lazy_c(lazy_c) { }
		geometry_representation operator () (const oriented_area_groups & /*oriented_areas*/) const {
			return nef_polyhedron_3::EMPTY;
		}
		geometry_representation operator () (const extrusion_information & ext) const {
			return extrusion_to_nef(ext, lazy_c()).interior();
		}
		geometry_representation operator () (const nef_polyhedron_3 & nef) const {
			return nef; 
		}
	};
	geometry = boost::apply_visitor(visitor(lazy_c), geometry);
}

} // namespace solid_geometry