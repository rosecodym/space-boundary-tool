#include "precompiled.h"

#include "surface_pair.h"

namespace blocking {

namespace impl {

bool surface_pair::drape_hits_other_plane() const {
	auto drape = base().drape();
	auto target_plane = other().backing_plane();
	return boost::find_if(drape, [&target_plane](const ray_3 & r) {
		return 
			!target_plane.has_on(r.point(0)) &&
			CGAL::do_intersect(target_plane, r.opposite());
	}) != drape.end();
}

bool surface_pair::contributes_to_envelope() const {
	return
		!is_self() &&
		!are_perpendicular() &&
		(!are_parallel() || // if they're parallel...
			(opposite_senses() && other_is_above_base() && areas_intersect())) &&
		(are_parallel() || // if they're not parallel...
			(other_in_correct_halfspace() &&
			drape_hits_other_plane()));
}

double surface_pair::dihedral_angle(const plane_3 & from, const plane_3 & to) const {
	// from wikipedia "dihedral angle"
	// this funkiness is all to get a sign on the angle
	if (from.orthogonal_vector() * to.orthogonal_vector() == 0) {
		return 3.14156 / 2;
	}

	CGAL::Object b2_obj = CGAL::intersection(from, to);
	const line_3 * b2_ptr = CGAL::object_cast<line_3>(&b2_obj);
	if (b2_ptr == nullptr) {
		return 0.0;
	}
	vector_3 b2 = b2_ptr->to_vector();
	vector_3 b1 = from.base1();
	if (b1.direction() == b2.direction() || b1.direction() == -b2.direction()) {
		b1 = from.base2();
	}
	vector_3 b3 = to.base1();
	if (b3.direction() == b2.direction() || b3.direction() == -b2.direction()) {
		b3 = to.base2();
	}
	return atan2(
		CGAL::to_double((sqrt(b2.squared_length()) * b1) * CGAL::cross_product(b2, b3)),
		CGAL::to_double(CGAL::cross_product(b1, b2) * CGAL::cross_product(b2, b3)));
}

double surface_pair::relative_height_at(const point_2 & p) const {
	// source: my notebook
	vector_3 base_vec = base().orientation().direction().to_vector();
	vector_3 other_vec = other().orientation().direction().to_vector();
	point_2 intr_point = other().parallel_plane_through_origin().to_2d(CGAL::ORIGIN + CGAL::cross_product(base_vec, other_vec));
	double point_angle = atan2(CGAL::to_double(p.x()), CGAL::to_double(p.y()));
	double intr_angle = atan2(CGAL::to_double(intr_point.x()), CGAL::to_double(intr_point.y()));
	double local_x_angle_scale = sin(CGAL::to_double(point_angle - intr_angle));

	return
		CGAL::to_double(other().height()) / cos(rotation) + // z component
		CGAL::to_double((p - CGAL::ORIGIN).squared_length()) * tan(local_x_angle_scale * rotation); // xy component
}

} // namespace impl

} // namespace blocking