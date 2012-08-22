#include "precompiled.h"

#include "equality_context.h"
#include "poly_builder.h"
#include "sbt-core.h"
#include "simple_face.h"
#include "solid_conversion_operations.h"

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

void print_halffacet(nef_halffacet_handle h) {
	for (auto cycle = h->facet_cycles_begin(); cycle != h->facet_cycles_end(); ++cycle) {
		NOTIFY_MSG("[cycle]\n");
		nef_polyhedron_3::SHalfedge_around_facet_const_circulator start(cycle);
		nef_polyhedron_3::SHalfedge_around_facet_const_circulator end(cycle);
		CGAL_For_all(start, end) {
			PRINT_POINT_3(start->source()->center_vertex()->point());
			NOTIFY_MSG("\n");
		}
	}
}

std::vector<simple_face> faces_from_brep(const brep & b, equality_context * c) {
	PRINT_SOLIDS("Creating face list from brep with %u faces.\n", b.face_count);
	std::vector<simple_face> res;
	std::transform(b.faces, b.faces + b.face_count, std::back_inserter(res), [c](const face & f) { return simple_face(f, c); });
	PRINT_SOLIDS("Face list created (%u faces).\n", res.size());
	return res;
}

extrusion_information get_extrusion_information(const extruded_area_solid & e, equality_context * c) {
	return std::make_tuple(simple_face(e.area, c), geometry_common::normalize(c->snap(direction_3(e.ext_dx, e.ext_dy, e.ext_dz)).to_vector()) * e.extrusion_depth);
}

enum face_status { FLIP = -1, NOT_DECIDED = 0, OK = 1};

std::tuple<size_t, face_status> find_root(const std::vector<simple_face> & all_faces, const std::vector<int> & group_memberships, int group) {
	size_t root_ix;
	face_status root_status = NOT_DECIDED;
	for (root_ix = 0; root_ix < all_faces.size(); ++root_ix) {
		if (group_memberships[root_ix] == group) {
			ray_3 shoot(all_faces[root_ix].average_outer_point(), all_faces[root_ix].plane().orthogonal_direction());
			bool found_intersection = false;
			bool found_intersection_reversed = false;
			for (size_t i = 0; i < all_faces.size(); ++i) {
				if (i != root_ix && group_memberships[i] == group) {
					if (!found_intersection && CGAL::do_intersect(shoot, all_faces[i].plane())) {
						found_intersection = true;
					}
					if (!found_intersection_reversed && CGAL::do_intersect(shoot.opposite(), all_faces[i].plane())) {
						found_intersection_reversed = true;
					}
					if (found_intersection && found_intersection_reversed) {
						break;
					}
				}
			}
			if (!found_intersection) {
				root_status = OK;
				break;
			}
			if (!found_intersection_reversed) {
				root_status = FLIP;
				break;
			}
		}
	}
	return std::make_tuple(root_ix, root_status);
}

enum face_relationship { MISMATCH = -1, NOT_CONNECTED = 0, MATCH = 1};

std::vector<simple_face> fix_orientations_for_group(
	const std::vector<simple_face> & all_faces, 
	const std::vector<int> & group_memberships,
	int group,
	const std::vector<std::vector<face_relationship>> & relationships)
{
	size_t root_ix;
	face_status root_status;
	std::tie(root_ix, root_status) = find_root(all_faces, group_memberships, group);

	std::vector<face_status> statuses(all_faces.size(), NOT_DECIDED);

	std::queue<std::tuple<size_t, face_status>> process_queue;
	process_queue.push(std::make_tuple(root_ix, root_status));

	while (!process_queue.empty()) {
		size_t next_ix;
		face_status next_status;
		std::tie(next_ix, next_status) = process_queue.front();
		process_queue.pop();
		if (statuses[next_ix] == NOT_DECIDED) {
			statuses[next_ix] = next_status;
			for (size_t i = 0; i < relationships.size(); ++i) {
				if (statuses[i] == NOT_DECIDED && relationships[next_ix][i] != NOT_CONNECTED) {
					process_queue.push(std::make_tuple(i, (face_status)(relationships[next_ix][i] * next_status)));
				}
			}
		}
	}

	std::vector<simple_face> res;
	for (size_t i = 0; i < all_faces.size(); ++i) {
		if (group_memberships[i] == group) {
			res.push_back(statuses[i] == FLIP ? all_faces[i].reversed() : all_faces[i]);
		}
	}

	return res;
}

std::vector<std::vector<simple_face>> to_simple_faces(std::vector<simple_face> && all_faces) {

	struct segment_comparator : public std::unary_function<segment_3, bool> {
		bool operator () (const segment_3 & a, const segment_3 & b) const {
			return a.source() == b.source() ? a.target() < b.target() : a.source() < b.source();
		}
	};
	
	PRINT_SOLIDS("Entered to_simple_faces.\n");

	std::vector<std::vector<face_relationship>> relationships(all_faces.size(), std::vector<face_relationship>(all_faces.size(), NOT_CONNECTED));

	std::map<segment_3, size_t, segment_comparator> edge_memberships;

	boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS> graph;

	for (size_t face_ix = 0; face_ix < all_faces.size(); ++face_ix) {
		boost::for_each(all_faces[face_ix].all_edges_voids_reversed(), [&edge_memberships, &relationships, &graph, face_ix](const segment_3 & e) {
			bool opposite = false;
			auto exists = edge_memberships.find(e);
			if (exists == edge_memberships.end()) {
				exists = edge_memberships.find(e.opposite());
				opposite = true;
			}
			if (exists != edge_memberships.end()) {
				size_t match_ix = exists->second;
				relationships[face_ix][match_ix] = relationships[match_ix][face_ix] = opposite ? MATCH : MISMATCH;
				boost::add_edge(face_ix, match_ix, graph);
			}
			else {
				edge_memberships.insert(std::make_pair(e, face_ix));
			}
		});
	}
	
	std::vector<int> group_memberships(all_faces.size());
	int group_count = boost::connected_components(graph, &group_memberships[0]);

	std::vector<std::vector<simple_face>> res;
	for (int i = 0; i < group_count; ++i) {
		res.push_back(fix_orientations_for_group(all_faces, group_memberships, i, relationships));
	}
	
	PRINT_SOLIDS("Exiting to_simple_faces.\n");

	return res;
}

bool simple_faces_are_nef_representable(const std::vector<std::vector<simple_face>> & volume_groups) {
	return boost::find_if(volume_groups, [](const std::vector<simple_face> & group) {
		return boost::find_if(group, [](const simple_face & f) {
			return !f.inners().empty();
		}) != group.end(); }) == volume_groups.end();
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

	PRINT_SOLIDS("Converting a nef volume to an oriented face group.");

	nef_polyhedron_3::Shell_entry_const_iterator sit;
	face_generator fgen(c);
	nef.visit_shell_objects(nef_sface_handle(v->shells_begin()), fgen);

	PRINT_SOLIDS("Got %u oriented faces from this volume.\n", fgen.get_faces().size());
	
	return fgen.get_faces();
}

oriented_area_groups nef_to_oriented_area_groups(const nef_polyhedron_3 & nef, equality_context * c) {
	PRINT_SOLIDS("Converting a nef polyhedron to oriented area groups.\n");
	oriented_area_groups g;
	nef_volume_handle v;
	CGAL_forall_volumes(v, nef) {
		if (v->mark()) {
			PRINT_SOLIDS("A nef volume is marked.\n");
			g.push_back(to_oriented_face_group(nef, v, c));
		}
		else {
			PRINT_SOLIDS("A nef volume is not marked.\n");
		}
	}
	PRINT_SOLIDS("Processed all nef volumes.\n");
	return g;
}

} // namespace

multiview_solid::multiview_solid(const solid & s, equality_context * c) {
	if (s.rep_type == REP_BREP) {
		geometry = to_simple_faces(faces_from_brep(s.rep.as_brep, c));
	}
	else if (s.rep_type == REP_EXT) {
		geometry = get_extrusion_information(s.rep.as_ext, c);
	}
	else if (FLAGGED(SBT_EXPENSIVE_CHECKS)) {
		ERROR_MSG("An incoming interface solid had no representation type.\n");
		abort();
	}
	validate();
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
		std::vector<multiview_solid> operator () (const simple_face_groups & simple_faces) const {
			std::vector<multiview_solid> res;
			boost::transform(simple_faces, std::back_inserter(res), [](const std::vector<simple_face> & group) {
				return multiview_solid(group);
			});
			return res;
		}
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
		geometry_representation operator () (const simple_face_groups & simple_faces) const {
			oriented_area_groups g(1);
			boost::transform(simple_faces.front(), std::back_inserter(g.front()), [this](const simple_face & f) {
				return oriented_area(f, c);
			});
			return g;
		}
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
	if (FLAGGED(SBT_EXPENSIVE_CHECKS) && groups.size() != 1) {
		ERROR_MSG("\nA solid had %u oriented face groups.\n", groups.size());
		abort();
	}
	return groups.front();
}

void multiview_solid::subtract(const multiview_solid & other, equality_context * c) {
	PRINT_ELEMENTS("Subtracting solid %x from solid %x.\n", this, &other);
	if (is_nef_representable() && other.is_nef_representable()) {
		PRINT_SOLIDS("Subtraction is possible.\n");
		convert_to_nef([c]() { return c; });
		other.convert_to_nef([c]() { return c; });
		PRINT_SOLIDS("Solids converted to nef.\n");
		boost::get<nef_polyhedron_3>(geometry) -= boost::get<nef_polyhedron_3>(other.geometry);
	}
	PRINT_ELEMENTS("Solid subtraction completed.\n");
}

bool multiview_solid::is_nef_representable() const {
	struct visitor : public boost::static_visitor<bool> {
		bool operator () (const simple_face_groups & simple_faces) const {
			return boost::find_if(simple_faces, [](const std::vector<simple_face> & group) {
				return boost::find_if(group, [](const simple_face & f) {
					return !f.inners().empty();
				}) != group.end(); }) == simple_faces.end();
		}
		bool operator () (const oriented_area_groups & /*oriented_areas*/) const { return false; /*yeah, maybe, but it shouldn't happen and it's hard*/}
		bool operator () (const extrusion_information & /*extrusion_information*/) const { return true; }
		bool operator () (const nef_polyhedron_3 & /*nef*/) const { return true; }
	};
	return boost::apply_visitor(visitor(), geometry);
}

void multiview_solid::convert_to_nef(std::function<equality_context *(void)> lazy_c) const {
	struct visitor : public boost::static_visitor<geometry_representation> {
		std::function<equality_context *(void)> lazy_c;
		visitor(std::function<equality_context *(void)> lazy_c) : lazy_c(lazy_c) { }
		geometry_representation operator () (const simple_face_groups & simple_faces) const {
			nef_polyhedron_3 res;
			for (auto group = simple_faces.begin(); group != simple_faces.end(); ++group) {
				polyhedron_3 poly;
				poly_builder builder(*group);
				poly.delegate(builder);
				res += nef_polyhedron_3(poly);
			}
			return res.interior();
		}
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

void multiview_solid::print() const {
	struct visitor : public boost::static_visitor<> {
		void operator () (const simple_face_groups &) const { }
		void operator () (const oriented_area_groups & groups) const {
			boost::for_each(groups, [](const std::vector<oriented_area> & group) {
				NOTIFY_MSG("Multiview solid oriented area group:\n");
				boost::for_each(group, [](const oriented_area & oa) {
					oa.print();
					NOTIFY_MSG("converts to:\n");
					NOTIFY_MSG(oa.to_3d().front().to_string().c_str());
				});
				NOTIFY_MSG("End multiview solid oriented area group.\n");
			});
		}
		void operator () (const extrusion_information &) const { }
		void operator () (const nef_polyhedron_3 & nef) const {
			nef_halffacet_handle h;
			CGAL_forall_facets(h, nef) {
				if (h->mark()) {
					NOTIFY_MSG("[face]\n");
					print_halffacet(h);
				}
			}
		}
	};
	boost::apply_visitor(visitor(), geometry);
}

void multiview_solid::validate() const {
	struct validator : public boost::static_visitor<> {
		void operator () (const simple_face_groups & groups) const {
			if (groups.size() == 0) {
				ERROR_MSG("A multiview_solid represented as simple face groups had no faces.\n");
				abort();
			}
		}
		void operator () (const oriented_area_groups & groups) const { 
			if (groups.size() == 0) {
				ERROR_MSG("A multiview_solid represented as oriented area groups had no faces.\n");
				abort();
			}
		}
		void operator () (const extrusion_information &) const { }
		void operator () (const nef_polyhedron_3 &) const { }
	};
	if (FLAGGED(SBT_EXPENSIVE_CHECKS)) {
		boost::apply_visitor(validator(), geometry);
	}
}

} // namespace solid_geometry