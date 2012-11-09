#pragma once

#include "precompiled.h"

#include "block.h"
#include "element.h"
#include "equality_context.h"
#include "oriented_area.h"

namespace blocking {

namespace impl {

class surface_pair;

typedef boost::multi_array<surface_pair, 2> relations_grid;
	
class surface_pair {
private:
	const oriented_area * m_base;
	const oriented_area * m_other;
	equality_context * m_c3d;

	// A negative thickness cutoff signifies "infinite."
	double m_thickness_cutoff;
	double m_rotation;

	// These fields memoize comparisons.
	mutable boost::optional<bool> m_areas_match;
	mutable boost::optional<oriented_area> m_projection_onto_base;
	mutable boost::optional<oriented_area> m_base_minus_other;
	mutable boost::optional<oriented_area> m_base_intr_other;

	// This constructor is used to generate a surface pair opposite.
	surface_pair(
		const oriented_area * base, 
		const oriented_area * other,
		double thickness_cutoff,
		equality_context * context_3d,
		boost::optional<bool> areas_match)
		: m_base(base),
		  m_other(other),
		  m_c3d(context_3d),
		  m_thickness_cutoff(thickness_cutoff),
		  m_areas_match(areas_match)
	{ 
		m_rotation = dihedral_angle(
			base->parallel_plane_through_origin(), 
			other->parallel_plane_through_origin());
	}

	bool areas_match() const {
		if (!m_areas_match) {
			m_areas_match = oriented_area::areas_match(*m_base, *m_other);
		}
		return *m_areas_match;
	}

	bool near_enough() const {
		if (m_thickness_cutoff < 0.0) { return true; }
		auto height_diff = other().height() - base().height();
		return abs(CGAL::to_double(height_diff)) <= m_thickness_cutoff;
	}

	const oriented_area & get_projection_onto_base() const {
		if (!m_projection_onto_base) {
			m_projection_onto_base = m_base->project_onto_self(*m_other);
		}
		return *m_projection_onto_base;
	}

	const oriented_area & get_base_minus_other_projected() const {
		if (!m_base_minus_other) {
			m_base_minus_other = *m_base - get_projection_onto_base();
		}
		return *m_base_minus_other;
	}

	const oriented_area & get_base_intr_other_projected() const {
		if (!m_base_intr_other) {
			m_base_intr_other = *m_base * get_projection_onto_base();
		}
		return *m_base_intr_other;
	}

public:
	// The default constructor is present because these are used in
	// multiarrays.
	surface_pair() { }

	// A negative thickness_cutoff signifies "infinite."
	surface_pair(
		const oriented_area & base, 
		const oriented_area & other, 
		equality_context * context_3d,
		double thickness_cutoff = -1);

	const oriented_area & base() const { return *m_base; }
	const oriented_area & other() const { return *m_other; }

	bool is_self() const { return m_base == m_other; }
	bool are_parallel() const { return !is_self() && oriented_area::are_parallel(*m_base, *m_other); }
	bool is_orthogonal_translation() const { return are_parallel() && areas_match(); }
	bool opposite_senses() const { return base().sense() != other().sense(); }
	bool other_is_above_base() const { return base().height() > other().height() == base().sense(); }
	bool areas_intersect() const { return area::do_intersect(base().area_2d(), other().area_2d()); }
	bool other_in_correct_halfspace() const { 
		return other().any_point_in_halfspace(
			base().backing_plane().opposite(), 
			m_c3d); 
	}
	bool drape_hits_other_plane() const;
	bool contributes_to_envelope() const;
	
	bool are_perpendicular() const { 
		return oriented_area::are_perpendicular(*m_base, *m_other, EPS_MAGIC); 
	}

	const oriented_area & base_minus_other_projected() const {
		return get_base_minus_other_projected();
	}

	const oriented_area & base_intr_other_projected() const {
		return get_base_intr_other_projected();
	}

	polygon_2 projected_other_area() const {
		auto outer = get_projection_onto_base().area_2d().outer_bound();
		return outer ? *outer : polygon_2();
	}

	surface_pair opposite() const { 
		return surface_pair(
			m_other, 
			m_base, 
			m_thickness_cutoff, 
			m_c3d, 
			m_areas_match); 
	}
	block to_block(const element & e) const { return block(*m_base, *m_other, e); }
	block to_halfblock(const element & e) const { return block(*m_base, e); }

	double dihedral_angle(const plane_3 & from, const plane_3 & to) const;

	double relative_height_at(const point_2 & p) const;

	// A negative thickness_cutoff signifies "infinite."
	static relations_grid build_relations_grid(
		const std::vector<oriented_area> & faces,
		equality_context * context_3d, 
		double thickness_cutoff = -1);
};

class Surface_pair_envelope_traits : public CGAL::Arr_segment_traits_2<K> {

public:
	typedef CGAL::Arr_segment_traits_2<K>	traits_2;
	typedef traits_2::X_monotone_curve_2	x_monotone_curve_2;
	typedef surface_pair					Surface_3;
	typedef surface_pair					Xy_monotone_surface_3;

	class Make_xy_monotone_3 {
	public:
		template <typename OutputIterator>
		OutputIterator operator () (const Surface_3 & pair, bool /*is_lower*/, OutputIterator oi) const {
			if (pair.contributes_to_envelope()) {
				*oi++ = pair;
			}
			return oi;
		}
	};

	class Construct_projected_boundary_2 {
	public:
		template <typename OutputIterator>
		OutputIterator operator () (const Xy_monotone_surface_3 & pair, OutputIterator oi) const {
			if (!pair.are_perpendicular()) {
				polygon_2 projection = pair.projected_other_area();
				
				std::vector<x_monotone_curve_2> boundaries;

				auto edge = projection.edges_circulator();
				auto end = edge;
				CGAL_For_all(edge, end) {
					boundaries.push_back(x_monotone_curve_2((*edge).source(), (*edge).target()));
				}

				boost::transform(boundaries, oi, [&projection](const x_monotone_curve_2 & curve) -> CGAL::Object {
					bool is_right = curve.is_directed_right();
					bool clockwise = projection.is_clockwise_oriented();
					bool position = is_right != clockwise;
					return CGAL::make_object(std::make_pair(curve, position ? CGAL::ON_POSITIVE_SIDE : CGAL::ON_NEGATIVE_SIDE));
				});
			}
			return oi;
		}
	};

	class Construct_projected_intersections_2 {
	public:
		template <typename OutputIterator>
		OutputIterator operator () (const Xy_monotone_surface_3 &, const Xy_monotone_surface_3 &, OutputIterator oi) const {
			return oi;
		}
	};

	class Compare_z_at_xy_3 {
	public:
		CGAL::Comparison_result operator () (const point_2 & point, const Xy_monotone_surface_3 & s1, const Xy_monotone_surface_3 & s2) const {
			double d1 = s1.relative_height_at(point);
			double d2 = s2.relative_height_at(point);
			return s1.base().sense() ? CGAL::compare(d2, d1) : CGAL::compare(d1, d2);
		}
		CGAL::Comparison_result operator () (const x_monotone_curve_2 & curve, const Xy_monotone_surface_3 & s1, const Xy_monotone_surface_3 & s2) const {
			CGAL::Comparison_result res = (*this)(curve.left(), s1, s2);
			if (res == CGAL::EQUAL) {
				res = (*this)(curve.right(), s1, s2);
				if (res == CGAL::EQUAL) {
					res = (*this)(K().construct_midpoint_2_object()(curve.left(), curve.right()), s1, s2);
				}
			}
			return res;
		}
	};

	class Compare_z_at_xy_above_3 {

	public:

		// I just straight up stole this from the triangle code
		CGAL::Comparison_result operator() (const X_monotone_curve_2 & curve, const Xy_monotone_surface_3 & surf1, const Xy_monotone_surface_3 & surf2) const {

			if (oriented_area::are_parallel(surf1.other(), surf2.other()) && oriented_area::same_height(surf1.other(), surf2.other())) {
				return CGAL::EQUAL;
			}

			// now we must have 2 different non-vertical planes:
 			// plane1: a1*x + b1*y + c1*z + d1 = 0  , c1 != 0
 			// plane2: a2*x + b2*y + c2*z + d2 = 0  , c2 != 0

			const plane_3 & plane1 = surf1.other().parallel_plane_through_origin();
			const plane_3 & plane2 = surf2.other().parallel_plane_through_origin();

			NT a1 = plane1.a(), b1 = plane1.b(), c1 = plane1.c();
			NT a2 = plane2.a(), b2 = plane2.b(), c2 = plane2.c();

 			// our line is a3*x + b3*y + c3 = 0
 			// it is assumed that the planes intersect over this line
			const line_2 & line = curve.line(); 
			NT a3 = line.a(), b3 = line.b(), c3 = line.c();

			CGAL::Sign s1 = CGAL::sign((a2*a3+b2*b3)/c2-(a1*a3+b1*b3)/c1);
    
			// We only need to make sure that w is in the correct direction
			// (going from down to up)
			// the original segment endpoints p1=(x1,y1) and p2=(x2,y2)
			// are transformed to (v1,w1) and (v2,w2), so we need that w2 > w1
			// (otherwise the result should be multiplied by -1)
      
			const point_2 & p1 = curve.left();
			const point_2 & p2 = curve.right();
			NT x1 = p1.x(), y1 = p1.y(), x2 = p2.x(), y2 = p2.y();

			CGAL::Sign s2 = CGAL::sign(-b3*x1+a3*y1-(-b3*x2+a3*y2));
			
			return s1 * s2;
		}
	};

	class Compare_z_at_xy_below_3 {
	public:
		CGAL::Comparison_result operator () (const X_monotone_curve_2 & cv, const Xy_monotone_surface_3& surf1, const Xy_monotone_surface_3& surf2) const {
			Comparison_result left_res = Compare_z_at_xy_above_3()(cv, surf1, surf2);
			return CGAL::opposite(left_res);
		}
	};

	Make_xy_monotone_3 make_xy_monotone_3_object() const {
		return Make_xy_monotone_3();
	}

	Construct_projected_boundary_2 construct_projected_boundary_2_object() const {
		return Construct_projected_boundary_2();
	}

	Construct_projected_intersections_2 construct_projected_intersections_2_object() const {
		return Construct_projected_intersections_2();
	}

	Compare_z_at_xy_3 compare_z_at_xy_3_object() const {
		return Compare_z_at_xy_3();
	}

	Compare_z_at_xy_above_3 compare_z_at_xy_above_3_object() const {
		return Compare_z_at_xy_above_3();
	}

	Compare_z_at_xy_below_3 compare_z_at_xy_below_3_object() const {
		return Compare_z_at_xy_below_3();
	}

};

} // namespace impl

} // namespace build_blocks