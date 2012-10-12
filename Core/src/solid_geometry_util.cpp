#include "precompiled.h"

#include "poly_builder.h"
#include "simple_face.h"

#include "solid_geometry_util.h"

namespace solid_geometry {

namespace impl {

nef_polyhedron_3 extrusion_to_nef(const extrusion_information & ext, equality_context * c) {
	const simple_face & f = std::get<0>(ext);
	const vector_3 & extrusion = std::get<1>(ext);
	
	transformation_3 extrude(CGAL::TRANSLATION, extrusion);
	polyhedron_3 poly;
	poly_builder builder(f.outer(), extrude, c);
	poly.delegate(builder);
	nef_polyhedron_3 res(poly);
	boost::for_each(f.inners(), [&res, &extrude, c](const std::vector<point_3> & inner) {
		polyhedron_3 poly;
		poly_builder builder(inner, extrude, c);
		poly.delegate(builder);
		res -= nef_polyhedron_3(poly);
	});
	return res;
}

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

} // namespace impl

} // namespace impl