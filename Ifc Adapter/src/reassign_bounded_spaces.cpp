#include "precompiled.h"

#include "cgal-typedefs.h"
#include "geometry_common.h"
#include "number_collection.h"
#include "sbt-ifcadapter.h"

#include "reassign_bounded_spaces.h"

namespace {

	typedef number_collection<K> q_collection;

	template<class HDS, class PointT>
	class shell_builder : public CGAL::Modifier_base<HDS> {

	private:

		std::vector<PointT> points;
		std::vector<std::vector<size_t>> r_indices;

	public:

		shell_builder(const solid & s, q_collection & qs) {
			if (s.rep_type == REP_BREP) {
				const brep & b = s.rep.as_brep;
				for (size_t i = 0; i < b.face_count; ++i) {
					r_indices.push_back(std::vector<size_t>());
					for (size_t j = 0; j < b.faces[i].outer_boundary.vertex_count; ++j) {
						size_t index;
						q_point_3 qp = qs.request_point(
							CGAL::to_double(b.faces[i].outer_boundary.vertices[j].x),
							CGAL::to_double(b.faces[i].outer_boundary.vertices[j].y),
							CGAL::to_double(b.faces[i].outer_boundary.vertices[j].z));
						for (index = 0; index < points.size(); ++index) {
							if (points[index] == qp) {
								r_indices.back().push_back(index);
								break;
							}
						}
						if (index == points.size()) {
							points.push_back(qp);
							r_indices.back().push_back(index);
						}
					}
				}
			}
			else if (s.rep_type == REP_EXT) {
				const extruded_area_solid & ext = s.rep.as_ext;
				q_vector_3 ext_vec = qs.request_direction(ext.ext_dx, ext.ext_dy, ext.ext_dz).to_vector();
				q_NT mag = qs.request_height(sqrt(CGAL::to_double(ext_vec.squared_length())));
				ext_vec = ext_vec / mag;
				q_transformation_3 extrusion(CGAL::TRANSLATION, ext_vec * ext.extrusion_depth);
				points.reserve(ext.area.outer_boundary.vertex_count * 2);
				for (size_t i = 0; i < ext.area.outer_boundary.vertex_count; ++i) {
					points.push_back(qs.request_point(
						ext.area.outer_boundary.vertices[i].x,
						ext.area.outer_boundary.vertices[i].y,
						ext.area.outer_boundary.vertices[i].z));
				}
				for (size_t i = 0; i < ext.area.outer_boundary.vertex_count; ++i) {
					points.push_back(points[i].transform(extrusion));
				}

				r_indices.push_back(std::vector<size_t>());
				for (size_t i = 0; i < ext.area.outer_boundary.vertex_count; ++i) {
					r_indices.back().push_back(i);
				}
				r_indices.push_back(std::vector<size_t>());
				for (size_t i = ext.area.outer_boundary.vertex_count - 1; i >= 0; --i) {
					r_indices.back().push_back(i + ext.area.outer_boundary.vertex_count);
				}

				for (size_t i = 0; i < ext.area.outer_boundary.vertex_count; ++i) {
					r_indices.push_back(std::vector<size_t>());
					r_indices.back().push_back(i);
					r_indices.back().push_back(ext.area.outer_boundary.vertex_count - 1 - i);
					r_indices.back().push_back(i != ext.area.outer_boundary.vertex_count - 1 ? (ext.area.outer_boundary.vertex_count - 2 - i) : (ext.area.outer_boundary.vertex_count - 1));
					r_indices.back().push_back((i + 1) % ext.area.outer_boundary.vertex_count);
				}

			}
			else {
				assert(s.rep_type == REP_BREP || s.rep_type == REP_EXT);
			}
		}

		void operator () (HDS & hds) {
			CGAL::Polyhedron_incremental_builder_3<HDS> b(hds, true);
			b.begin_surface(points.size(), r_indices.size());
			for (auto p = points.begin(); p != points.end(); ++p) {
				b.add_vertex(*p);
			}
			for (auto f = r_indices.begin(); f != r_indices.end(); ++f) {
				b.add_facet(f->begin(), f->end());
			}
			b.end_surface();
		}

	};

	class shell_explorer {
		bool first;
		const q_nef_polyhedron_3 & poly;
		q_nef_polyhedron_3::Vertex_const_handle v_min;

		public:
		shell_explorer(const q_nef_polyhedron_3 & poly) : first(true), poly(poly) { }

		void visit(q_nef_polyhedron_3::Vertex_const_handle v) {
			if (first || CGAL::lexicographically_xyz_smaller(v->point(),v_min->point())) {
				v_min = v;
				first=false;
			}
		}

		void visit(q_nef_polyhedron_3::Halfedge_const_handle) { }
		void visit(q_nef_polyhedron_3::Halffacet_const_handle) { }
		void visit(q_nef_polyhedron_3::SHalfedge_const_handle) { }
		void visit(q_nef_polyhedron_3::SHalfloop_const_handle) { }
		void visit(q_nef_polyhedron_3::SFace_const_handle) { }

		q_nef_polyhedron_3::Vertex_const_handle & minimal_vertex() { return v_min; }
	};


	q_nef_polyhedron_3 build_geometry(space_info * space, q_collection & qs) {
		q_polyhedron_3 qpoly;
		shell_builder<q_polyhedron_3::HDS, q_point_3> builder(space->geometry, qs);
		qpoly.delegate(builder);
		return q_nef_polyhedron_3(qpoly);
	}

	point_3 calculate_midpoint(const std::vector<point_3> & points) {
		NT xtotal = std::accumulate(points.begin(), points.end(), NT(0.0), [](NT curr, const point_3 & p) { return curr + p.x(); });
		NT ytotal = std::accumulate(points.begin(), points.end(), NT(0.0), [](NT curr, const point_3 & p) { return curr + p.y(); });
		NT ztotal = std::accumulate(points.begin(), points.end(), NT(0.0), [](NT curr, const point_3 & p) { return curr + p.z(); });
		return point_3(xtotal / (int)points.size(), ytotal / (int)points.size(), ztotal / (int)points.size());
	}

	point_3 calculate_midpoint(space_boundary * boundary) {
		double xtotal = 0;
		double ytotal = 0;
		double ztotal = 0;
		for (size_t i = 0; i < boundary->geometry.vertex_count; ++i) {
			xtotal += boundary->geometry.vertices[i].x;
			ytotal += boundary->geometry.vertices[i].y;
			ztotal += boundary->geometry.vertices[i].z;
		}
		return point_3(xtotal / boundary->geometry.vertex_count, ytotal / boundary->geometry.vertex_count, ztotal / boundary->geometry.vertex_count);
	}

	point_3 calculate_midpoint(const std::vector<space_boundary *> & boundaries) {
		std::vector<point_3> midpoints;
		for (auto p = boundaries.begin(); p != boundaries.end(); ++p) {
			midpoints.push_back(calculate_midpoint(*p));
		}
		return calculate_midpoint(midpoints);
	}

	q_point_3 convert_point(const point_3 & p) {
		return q_point_3(CGAL::to_double(p.x()), CGAL::to_double(p.y()), CGAL::to_double(p.z()));
	}

	// i fucking hate these
	// i really am considering creating a "useful nef polyhedron" wrapper library
	// why is the interface completely unusable
#pragma warning (push)
#pragma warning (disable:4702) // unreachable code, because the only way i know how to get at the info i need is with these dumb macros
	q_point_3 outside_point(const q_nef_polyhedron_3 & poly) {
		shell_explorer explorer(poly);
		q_nef_polyhedron_3::Volume_const_iterator vit;
		CGAL_forall_volumes(vit, poly) {
			q_nef_polyhedron_3::Shell_entry_const_iterator sit;
			CGAL_forall_shells_of(sit, vit) {
				poly.visit_shell_objects(q_nef_polyhedron_3::SFace_const_handle(sit), explorer);
				q_point_3 min_point = explorer.minimal_vertex()->point();
				return q_point_3(min_point.x() * 2, min_point.y() * 2, min_point.z() * 2);
			}
			break;
		}
		assert(false);
		return q_point_3();
	}
#pragma warning (pop)

	bool shell_contains(const q_nef_polyhedron_3 & shell, const q_point_3 & point) {
		CGAL::Object_handle obj = shell.locate(point);
		q_nef_polyhedron_3::Volume_const_handle point_vol;
		if (CGAL::assign(point_vol, obj)) {
			obj = shell.locate(outside_point(shell));
			q_nef_polyhedron_3::Volume_const_handle outside_vol;
			if (CGAL::assign(outside_vol, obj)) {
				return point_vol != outside_vol;
			}
			else {
				assert(CGAL::assign(outside_vol, obj));
			}
		}
		else {
			assert(CGAL::assign(point_vol, obj));
		}
		return false;
	}

}

void reassign_bounded_spaces(
	size_t loaded_space_count,
	space_info ** loaded_spaces, 
	size_t /*calculated_space_count*/, 
	space_info ** /*calculated_space_info*/,
	size_t sb_count,
	space_boundary ** sbs)
{

	q_collection qs(0.0005);

	std::vector<std::pair<q_nef_polyhedron_3, std::string>> loaded_space_geometry;
	for (size_t i = 0; i < loaded_space_count; ++i) {
		loaded_space_geometry.push_back(std::make_pair(build_geometry(loaded_spaces[i], qs), loaded_spaces[i]->id));
	}

	std::map<std::string, std::vector<space_boundary *>> calculated_spaces;

	for (size_t i = 0; i < sb_count; ++i) {
		if (sbs[i]->bounded_space != nullptr) {
			auto res = calculated_spaces.find(sbs[i]->bounded_space->id);
			if (res == calculated_spaces.end()) {
				calculated_spaces.insert(std::make_pair(sbs[i]->bounded_space->id, std::vector<space_boundary *>()));
				calculated_spaces[sbs[i]->bounded_space->id].push_back(sbs[i]);
			}
			else {
				res->second.push_back(sbs[i]);
			}
		}
	}

	for (auto p = calculated_spaces.begin(); p != calculated_spaces.end(); ++p) {
		q_point_3 midpoint = convert_point(calculate_midpoint(p->second));
		bool found_space_match = false;
		for (auto q = loaded_space_geometry.begin(); q != loaded_space_geometry.end(); ++q) {
			if (shell_contains(q->first, midpoint)) {
				found_space_match = true;
				for (auto r = p->second.begin(); r != p->second.end(); ++r) {
					strncpy((*r)->bounded_space->id, q->second.c_str(), SPACE_ID_MAX_LEN);
				}
				break;
			}
		}
		assert(found_space_match);
	}
}