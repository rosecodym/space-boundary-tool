#include "precompiled.h"

#include "surface_pair.h"

namespace blocking {

namespace impl {	
	
surface_pair::surface_pair(
	const oriented_area * base, 
	const oriented_area * other,
	double thickness_cutoff,
	equality_context * context_2d,
	equality_context * context_3d,
	boost::optional<bool> areas_match)
	: m_base(base),
		m_other(other),
		m_c2d(context_2d),
		m_c3d(context_3d),
		m_thickness_cutoff(thickness_cutoff),
		m_areas_match(areas_match),
		m_rotation(dihedral_angle(
			base->parallel_plane_through_origin(), 
			other->parallel_plane_through_origin())) { }

surface_pair::surface_pair(
	const oriented_area & base, 
	const oriented_area & other, 
	equality_context * context_3d,
	double thickness_cutoff)
	: m_base(&base), 
	  m_other(&other), 
	  m_c2d(nullptr),
	  m_c3d(context_3d), 
	  m_thickness_cutoff(thickness_cutoff),
	  m_rotation(dihedral_angle(
		base.parallel_plane_through_origin(), 
		other.parallel_plane_through_origin())) { }

const area & surface_pair::get_projection_onto_base() const {
	if (!m_projection_onto_base) {
		m_projection_onto_base = m_base->project_onto_self(*m_other).area_2d();
		if (m_c2d) {
			m_projection_onto_base = m_projection_onto_base->snap(m_c2d);
		}
	}
	assert(m_projection_onto_base);
	return *m_projection_onto_base;
}

const area & surface_pair::get_base_minus_other_projected() const {
	if (!m_base_minus_other) {
		area base_area = m_base->area_2d();
		if (m_c2d) { base_area = m_base->area_2d().snap(m_c2d); }
		m_base_minus_other = base_area - get_projection_onto_base();
	}
	assert(m_base_minus_other);
	return *m_base_minus_other;
}

const area & surface_pair::get_base_intr_other_projected() const {
	if (!m_base_intr_other) {
		area base_area = m_base->area_2d();
		if (m_c2d) { base_area = m_base->area_2d().snap(m_c2d); }
		m_base_intr_other = base_area * get_projection_onto_base();
	}
	assert(m_base_intr_other);
	return *m_base_intr_other;
}

bool surface_pair::drape_hits_other_plane() const {
	auto drape = base().drape();
	auto target_plane = other().backing_plane();
	return boost::find_if(drape, [&target_plane](const ray_3 & r) {
		return 
			!target_plane.has_on(r.point(0)) &&
			CGAL::do_intersect(target_plane, r.opposite());
	}) != drape.end();
}

surface_pair::envelope_contribution 
surface_pair::contributes_to_envelope() const
{
	if (!m_env_contribution) {
		if (is_self() || are_perpendicular()) { m_env_contribution = NONE; }
		else if (
			are_parallel() &&
			opposite_senses() &&
			other_is_above_base() &&
			near_enough() &&
			areas_intersect())
		{
			m_env_contribution = PARALLEL;
		}
		else if (
			!are_parallel() &&
			other_in_correct_halfspace() &&
			drape_hits_other_plane())
		{
			m_env_contribution = NONPARALLEL;
		}
		else {
			m_env_contribution = NONE;
		}
	}
	return *m_env_contribution;
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
	// I've kind of forgotten how this works. It was supposed to find the
	// height by first identifying the distance between the surfaces along a
	// line through the origin and then modifying it based on the position of 
	// p, but since it doesn't use the height of the base surface anywhere
	// there's clearly a problem. What I think is happening is that since it's
	// only ever used to compare heights for surfaces pairs with the same base
	// the "missing base offset" never comes into play (since the error is
	// constant across all comparisons).
	auto dbl = [](const NT & n) { return CGAL::to_double(n); };
	vector_3 base_vec = base().orientation().direction().to_vector();
	vector_3 other_vec = other().orientation().direction().to_vector();

	point_2 intr_point = other().parallel_plane_through_origin().to_2d(CGAL::ORIGIN + CGAL::cross_product(base_vec, other_vec));
	double point_angle = atan2(dbl(p.x()), dbl(p.y()));
	double intr_angle = atan2(dbl(intr_point.x()), dbl(intr_point.y()));
	double local_x_angle_scale = sin(dbl(point_angle - intr_angle));

	double dist_at_ori = dbl(other().height()) / cos(m_rotation);
	double p_dist_from_ori = dbl((p - CGAL::ORIGIN).squared_length());
	double p_effect_factor = tan(local_x_angle_scale * m_rotation);

	return dist_at_ori + p_dist_from_ori * p_effect_factor;
}

relations_grid surface_pair::build_relations_grid(
	const std::vector<oriented_area> & faces,
	equality_context * context_3d, 
	double max_thickness_cutoff)
{
	relations_grid res(boost::extents[faces.size()][faces.size()]);
	for (size_t i = 0; i < faces.size(); ++i) {
		for (size_t j = i; j < faces.size(); ++j) {
			res[i][j] = surface_pair(
				faces[i], 
				faces[j], 
				context_3d, 
				max_thickness_cutoff);
			res[j][i] = res[i][j].opposite();
		}
	}
	return res;
}

} // namespace impl

} // namespace blocking