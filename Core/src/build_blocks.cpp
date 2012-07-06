#include "precompiled.h"

#include "area.h"
#include "cgal-util.h"
#include "element.h"
#include "geometry_common.h"
#include "printing-util.h"
#include "oriented_area.h"
#include "surface.h"

#define PRINT_BLOCKS(...) \
	do { \
		IF_FLAGGED(SBT_VERBOSE_BLOCKS) { \
			NOTIFY_MSG(__VA_ARGS__); \
		} \
	} \
	while (false);

extern sb_calculation_options g_opts;

void PRINT_BLOCKS_PZBS(const oriented_area & p) {
	IF_FLAGGED(SBT_VERBOSE_BLOCKS) {
		NOTIFY_MSG( "[<%f, %f, %f>%c @ %f]\n",
			CGAL::to_double(p.orientation().dx()),
			CGAL::to_double(p.orientation().dy()),
			CGAL::to_double(p.orientation().dz()),
			(p.sense() ? '+' : '-'),
			CGAL::to_double(p.height()));
		p.area_2d().print_with(g_opts.notify_func);
	}
}

namespace {

/********************************************************************************************************
 * SECTION THE FIRST: RELATIVE_PZBS
 ********************************************************************************************************/

double dihedral_angle(const plane_3 & from, const plane_3 & to, equality_context * context) {
	// from wikipedia "dihedral angle"
	assert(!from.is_degenerate());
	assert(!to.is_degenerate());

	if (context->are_effectively_perpendicular(from.orthogonal_vector(), to.orthogonal_vector())) {
		return 3.14159 / 2;
	}

	CGAL::Object b2_obj = CGAL::intersection(from, to);
	const line_3 * b2_ptr = CGAL::object_cast<line_3>(&b2_obj);
	if (b2_ptr == nullptr) {
		assert(from == to || from == to.opposite());
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

// this is the surface type for the envelope calculation
class relative_pzbs {

private:
	equality_context * context;

	const oriented_area * _reference;	// mutable pointer to immutable object
	const oriented_area & _me;			// reference to immutable object
	polygon_2 my_outer;
	double _rotation_from_base;

	relative_pzbs & operator = (const relative_pzbs & src);

public:
	enum relevance_t {
		PARALLEL_SAME_AREAS,
		PARALLEL,
		NONPARALLEL,
		IRRELEVANT
	};

	relative_pzbs(const oriented_area & me, equality_context * context)
		: _reference(nullptr), _me(me), my_outer(me.area_2d().outer_bound()), _rotation_from_base(0.0), context(context)
	{ }

	void set_reference(const relative_pzbs & other) {
		_reference = &other._me;
		_rotation_from_base = dihedral_angle(_reference->base_plane(), _me.base_plane(), context);
	}

	const oriented_area & pzbs() const { return _me; }
	const oriented_area & relative_to() const { return *_reference; }
	bool is_vertical() const { return _rotation_from_base == 3.14159 / 2 || _rotation_from_base == -3.14159 / 2; }
	bool references_self() const { return _reference == &_me; }
	bool is_relevant() const { return !references_self() && !is_vertical(); }
	bool areas_match() const { return _me.area_2d() == _reference->area_2d(); }

	polygon_2 projection() const { 
		polygon_2 res = _reference->project_onto_self(my_outer, _me);
		if (!util::cgal::polygon_has_no_adjacent_duplicates(res, context)) {
			NOTIFY_MSG("[Bad polygon follows]\n");
			util::printing::print_polygon(g_opts.notify_func, res);
			SBT_ASSERT(false, "[Aborting - a relative pzbs projection had adjacent duplicate points.]\n");
		}
		return context->snap(res);
	}

	double relative_height_at(const point_2 & point) const {
		assert(!is_vertical());

		// i am making an assumption here about the orientation of something or other
		// if i had a clever way to find the signed angle i could dispense with the trig calls entirely
		vector_3 base_vec = relative_to().orientation()/*.direction()*/.vector();
		vector_3 target_vec = pzbs().orientation()/*.direction()*/.vector();
		point_2 intr_point = relative_to().base_plane().to_2d(CGAL::ORIGIN + CGAL::cross_product(base_vec, target_vec));
		NT r_point_2angle(atan2(CGAL::to_double(point.x()), CGAL::to_double(point.y())));
		NT intr_angle(atan2(CGAL::to_double(intr_point.x()), CGAL::to_double(intr_point.y())));
		NT local_x_angle_scale(sin(CGAL::to_double(r_point_2angle - intr_angle)));

		return
			CGAL::to_double(pzbs().height()) / cos(CGAL::to_double(_rotation_from_base)) + // z contribution
			CGAL::to_double(sqrt(point.x() * point.x() + point.y() * point.y())) * tan(CGAL::to_double(local_x_angle_scale * _rotation_from_base)); // xy contribution

	}

	relevance_t get_relevance() const {
		if (references_self()) {
			return IRRELEVANT;
		}
		if (is_vertical()) {
			return IRRELEVANT;
		}
		if (oriented_area::are_parallel(pzbs(), relative_to())) {
			if ((relative_to().sense() != pzbs().sense()) && (relative_to().height() > pzbs().height() == relative_to().sense())) {
				if (pzbs().area_2d() == relative_to().area_2d()) {
					return PARALLEL_SAME_AREAS;
				}
				if (area::do_intersect(pzbs().area_2d(), relative_to().area_2d())) {
					return PARALLEL;
				}
				return IRRELEVANT;
			}
			else {
				return IRRELEVANT;
			}
		}
		plane_3 actual_plane = pzbs().backing_plane();
		auto drape = relative_to().drape_rays();
		if (boost::find_if(drape, [&actual_plane](const ray_3 & r) -> bool {
			return !actual_plane.has_on(r.point(0)) && CGAL::do_intersect(actual_plane, r);
		}) != drape.end())
		{
			return NONPARALLEL;
		}
		return IRRELEVANT;
	}
};

typedef std::pair<std::vector<relative_pzbs>::iterator, relative_pzbs::relevance_t> rel_pair;

// this makes the envelope magic work
// a couple of things make life way easy:
// first, we only care about faces in the final arrangement, and
// second, we know that surfaces only intersect on their edges
class Env_pzbs_surface_traits_3 : public CGAL::Arr_segment_traits_2<K> {

public:

	typedef CGAL::Arr_segment_traits_2<K>	traits_2;
	typedef traits_2::X_monotone_curve_2	x_monotone_curve_2;

	typedef rel_pair						Surface_3;
	typedef relative_pzbs					Xy_monotone_surface_3;

	class Make_xy_monotone_3 {

	public:
		template <class OutputIterator>
		OutputIterator operator () (const Surface_3 & surf, bool /*is_lower*/, OutputIterator oi) const {

			if (surf.second != relative_pzbs::IRRELEVANT) {
				PRINT_BLOCKS("[generating xy monotone surface]\n");
				PRINT_BLOCKS_PZBS(surf.first->pzbs());
				*oi++ = *surf.first;
			}

			return oi;

		}

	};

	class Construct_projected_boundary_2 {

	public:
		template <class OutputIterator>
		OutputIterator operator () (const Xy_monotone_surface_3 & surf, OutputIterator oi) const {

			if (surf.is_vertical()) {
				return oi;
			}

			polygon_2 projection = surf.projection();
			IF_FLAGGED(SBT_VERBOSE_BLOCKS) {
				NOTIFY_MSG( "projected to:\n");
				util::printing::print_polygon(g_opts.notify_func, projection);
			}
			SBT_ASSERT(projection.is_simple(), "[Aborting - encountered a projection to a non-simple polygon during an envelope calculation.]\n");

			std::vector<x_monotone_curve_2> boundaries;

			auto q = projection.vertices_begin();
			auto p = q++;
			for (; q != projection.vertices_end(); ++p, ++q) {
				SBT_ASSERT(!equality_context::are_effectively_same(*p, *q, 0.001), "[Aborting - tried to create an effectively degenerate x-monotone curve.]\n");
				boundaries.push_back(x_monotone_curve_2(*p, *q));
			}
			boundaries.push_back(x_monotone_curve_2(*p, *projection.vertices_begin()));

			for (auto p = boundaries.begin(); p != boundaries.end(); ++p) {
				bool is_right = p->is_directed_right();
				bool clockwise = projection.is_clockwise_oriented();
				bool position = is_right != clockwise;
				*oi++ = CGAL::make_object(std::make_pair(*p, position ? CGAL::ON_POSITIVE_SIDE : CGAL::ON_NEGATIVE_SIDE));
			}
				
			return oi;
		}

	};

	// this one is easy mode because we know beforehand that every intersection exists on an edge, so it will get handled elsewhere
	class Construct_projected_intersections_2 {

	public:

		template <class OutputIterator>
		OutputIterator operator () (const Xy_monotone_surface_3 &, const Xy_monotone_surface_3 &, OutputIterator oi) const {
			return oi;
		}

	};

	class Compare_z_at_xy_3 {

	public:

		CGAL::Comparison_result operator () (const point_2 & point, const Xy_monotone_surface_3 & s1, const Xy_monotone_surface_3 & s2) const {
			double d1 = s1.relative_height_at(point);
			double d2 = s2.relative_height_at(point);
			return s1.relative_to().sense() ? CGAL::compare(d2, d1) : CGAL::compare(d1, d2);
		}

		CGAL::Comparison_result operator () (const x_monotone_curve_2 & curve, const Xy_monotone_surface_3 & s1, const Xy_monotone_surface_3 s2) const {

			CGAL::Comparison_result res;
			res = (*this)(curve.left(), s1, s2);

			if (res == CGAL::EQUAL) {
				res = (*this)(curve.right(), s1, s2);
				if (res == CGAL::EQUAL)
				{
					res = (*this)(K().construct_midpoint_2_object()(curve.left(), curve.right()), s1, s2);
				}
			}

			return res;
		}
	};

	class Compare_z_at_xy_above_3 {

	public:

		// I just straight up stole this from the triangle code
		Comparison_result operator() (const X_monotone_curve_2 & curve, const Xy_monotone_surface_3 & surf1, const Xy_monotone_surface_3 & surf2) const {

			// a vertical surface cannot be defined in the infinitesimal region above
			// a curve
			assert(!surf1.is_vertical());
			assert(!surf2.is_vertical());

			if (oriented_area::are_parallel(surf1.pzbs(), surf2.pzbs()) && surf1.pzbs().height() == surf2.pzbs().height())
			{
				return CGAL::EQUAL;
			}

			// now we must have 2 different non-vertical planes:
 			// plane1: a1*x + b1*y + c1*z + d1 = 0  , c1 != 0
 			// plane2: a2*x + b2*y + c2*z + d2 = 0  , c2 != 0

			const plane_3 & plane1 = surf1.pzbs().base_plane();
			const plane_3 & plane2 = surf2.pzbs().base_plane();

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

/*******************************************************************************************************************
 * SECTION THE SECOND: GOOFY BLOCKS OPTIMIZATIONS BECAUSE ENVELOPES ARE TOO SLOW
 ******************************************************************************************************************/

// this thing basically just memoizes a bunch of comparisons
class special_case_finder {

private:
	enum relation {
		UNKNOWN = 0,
		TRUE,
		FALSE
	};

	std::shared_ptr<element> e;
	std::vector<oriented_area> faces;
	
	boost::multi_array<relation, 2> intersections;
	boost::multi_array<relation, 2> containment;
	boost::multi_array<relation, 2> matching;
	boost::multi_array<relation, 2> parallels;

	bool intersects(int i, int j) {
		if (i != j && intersections[i][j] == UNKNOWN) {
			intersections[i][j] = intersections[j][i] = area::do_intersect(faces[i].area_2d(), faces[j].area_2d()) ? TRUE : FALSE;
		}
		return intersections[i][j] == TRUE;
	}

	bool contains(int i, int j) {
		if (i != j && containment[i][j] == UNKNOWN) {
			containment[i][j] = faces[i].area_2d() >= faces[j].area_2d() ? TRUE : FALSE;
		}
		return containment[i][j] == TRUE;
	}

	bool matches(int i, int j) {
		if (i != j && matching[i][j] == UNKNOWN) {
			matching[i][j] = matching[j][i] = faces[i].area_2d() == faces[j].area_2d() ? TRUE : FALSE;
			if (FLAGGED(SBT_VERBOSE_BLOCKS)) {
				NOTIFY_MSG("Match %s:\n", matching[i][j] == TRUE ? "success" : "failure");
				PRINT_BLOCKS_PZBS(faces[i]);
				PRINT_BLOCKS_PZBS(faces[j]);
			}
		}
		return matching[i][j] == TRUE;
	}

	bool are_parallel(int i, int j) {
		if (i != j && parallels[i][j] == UNKNOWN) {
			parallels[i][j] = parallels[j][i] = oriented_area::are_parallel(faces[i], faces[j]) ? TRUE : FALSE;
		}
		return parallels[i][j] == TRUE;
	}

	template <class OutputIterator> bool check_for_tetrahedron(OutputIterator & oi);
	template <class OutputIterator> bool check_for_right_cuboid(OutputIterator & oi);
	template <class OutputIterator> bool check_for_hexahedral_prismatoid(OutputIterator & oi);

public:
	special_case_finder(std::shared_ptr<element> e, equality_context * c) 
		: e(e),
		faces(e->geometry().oriented_faces(c)),
		intersections(boost::extents[faces.size()][faces.size()]),
		containment(boost::extents[faces.size()][faces.size()]),
		matching(boost::extents[faces.size()][faces.size()]),
		parallels(boost::extents[faces.size()][faces.size()])
	{
		for (int i = 0; i < faces.size(); ++i) {
			for (int j = 0; j < faces.size(); ++j) {
				intersections[i][j] = UNKNOWN;
				containment[i][j] = UNKNOWN;
				matching[i][j] = UNKNOWN;
				parallels[i][j] = UNKNOWN;
			}
		}
		if (FLAGGED(SBT_VERBOSE_ELEMENTS)) {
			NOTIFY_MSG("Faces for element %s:\n", e->source().c_str());
			boost::for_each(faces, [](const oriented_area & f) { 
				NOTIFY_MSG("Hessian:\n");
				f.print();
				NOTIFY_MSG("3d:\n");
				PRINT_LOOP_3(f.to_3d().front().outer()); 
			});
		}
	}

	template <class OutputIterator>
	std::vector<int> process_and_get_remaining_indices(OutputIterator & oi) {
		if (check_for_tetrahedron(oi) || check_for_right_cuboid(oi) || check_for_hexahedral_prismatoid(oi)) { 
			return std::vector<int>(); 
		}
		NOTIFY_MSG(" no special cases match.");
		std::vector<int> results;
		// could do more here. maybe i will later.
		for (int i = 0; i < faces.size(); ++i) {
			bool found_para = false;
			for (int j = 0; j < faces.size(); ++j) {
				if (are_parallel(i, j)) {
					found_para = true;
					break;
				}
			}
			if (found_para) {
				results.push_back(i);
			}
			else {
				*oi++ = std::shared_ptr<surface>(new surface(faces[i], e));
			}
		}
		NOTIFY_MSG(" %u of %u faces require an envelope calculation", results.size(), faces.size());
		return results;
	}

};

template <class OutputIterator>
bool special_case_finder::check_for_tetrahedron(OutputIterator & oi) {
	if (faces.size() == 4) {
		std::transform(faces.begin(), faces.end(), oi, [this](const oriented_area & face) {
			return std::shared_ptr<surface>(new surface(face, e));
		});
		NOTIFY_MSG(" element is a tetrahedron.\n");
		return true;
	}
	return false;
}

template <class OutputIterator> 
bool special_case_finder::check_for_right_cuboid(OutputIterator & oi) {
	if (faces.size() == 6) {
		int matched[6] = { -1, -1, -1, -1, -1, -1 };
		for (int i = 0; i < 6; ++i) {
			if (matched[i] == -1) {
				for (int j = i + 1; j < 6; ++j) {
					if (matched[j] == -1 && are_parallel(i, j) && matches(i, j))
					{
						matched[i] = j;
						matched[j] = i;
						break;
					}
				}
			}
			if (matched[i] == -1) {
				return false;
			}
		}
		std::shared_ptr<surface> surfaces[6];
		std::transform(faces.begin(), faces.end(), surfaces, [this](const oriented_area & face) {
			return std::shared_ptr<surface>(new surface(face, e));
		});
		for (int i = 0; i < 5; ++i) {
			surface::set_other_sides(surfaces[i], surfaces[matched[i]]);
			PRINT_BLOCKS("[linking %s and %s]\n", surfaces[i]->guid().c_str(), surfaces[i]->opposite().lock()->guid().c_str());
		}
		std::copy(surfaces, surfaces + 6, oi);
		NOTIFY_MSG(" element is a right cuboid.\n");
		return true;
	}
	return false;
}

template <class OutputIterator> 
bool special_case_finder::check_for_hexahedral_prismatoid(OutputIterator & oi) {
	if (faces.size() == 6) {
		bool processed[6] = { false, false, false, false, false, false };
		int bases[2] = { -1, -1 };
		for (int i = 0; i < 6; ++i) {
			for (int j = i + 1; j < 6; ++j) {
				if (are_parallel(i, j) && matches(i, j)) {
					bases[0] = i;
					bases[1] = j;
					processed[i] = true;
					processed[j] = true;
					break;
				}
			}
			if (bases[0] != -1) {
				break;
			}
		}
		if (bases[0] == -1) {
			return false;
		}
		NOTIFY_MSG(" element is a hexahedral prismatoid.\n");
		std::shared_ptr<surface> a(new surface(faces[bases[0]], e));
		std::shared_ptr<surface> b(new surface(faces[bases[1]], e));
		surface::set_other_sides(a, b);
		*oi++ = a;
		*oi++ = b;
		for (int i = 0; i < 6; ++i) {
			if (!processed[i]) {
				for (int j = i + 1; j < 6; ++j) {
					if (!processed[j] && are_parallel(i, j)) {
						IF_FLAGGED(SBT_VERBOSE_BLOCKS) {
							NOTIFY_MSG( "[found parallel non-base pair - calculating intersection between]\n");
							faces[i].area_2d().print_with(g_opts.notify_func);
							NOTIFY_MSG( "[and]\n");
							faces[j].area_2d().print_with(g_opts.notify_func);
						}
						SBT_ASSERT(!faces[i].area_2d().is_empty() && !faces[j].area_2d().is_empty(), "[Aborting - one or both surfaces in a parallel non-base pair were empty.]\n");
						area intr = faces[i].area_2d() * faces[j].area_2d();
						PRINT_BLOCKS("[intersection calculated]\n");
						if (!intr.is_empty()) {
							PRINT_BLOCKS("[intersection is not empty]\n");
							std::shared_ptr<surface> intr_a(new surface(oriented_area(faces[i], intr), e));
							std::shared_ptr<surface> intr_b(new surface(oriented_area(faces[j], intr), e));
							surface::set_other_sides(intr_a, intr_b);
							*oi++ = intr_a;
							*oi++ = intr_b;
							IF_FLAGGED(SBT_VERBOSE_BLOCKS) {
								NOTIFY_MSG( "[calculating differences between]\n");
								faces[i].area_2d().print_with(g_opts.notify_func);
								NOTIFY_MSG( "[and]\n");
								faces[j].area_2d().print_with(g_opts.notify_func);
							}
							area left_a = faces[i].area_2d() - faces[j].area_2d();
							IF_FLAGGED(SBT_VERBOSE_BLOCKS) {
								NOTIFY_MSG( "[first difference calculated:]\n");
								left_a.print_with(g_opts.notify_func);
							}
							area left_b = faces[j].area_2d() - faces[i].area_2d();
							IF_FLAGGED(SBT_VERBOSE_BLOCKS) {
								NOTIFY_MSG( "[second difference calculated:]\n");
								left_b.print_with(g_opts.notify_func);
							}
							if (!left_a.is_empty()) { *oi++ = std::shared_ptr<surface>(new surface(oriented_area(faces[i], std::move(left_a)), e)); }
							if (!left_b.is_empty()) { *oi++ = std::shared_ptr<surface>(new surface(oriented_area(faces[j], std::move(left_b)), e)); }
						}
						else {
							PRINT_BLOCKS("[intersection is empty]\n");
							SBT_ASSERT(!faces[i].area_2d().is_empty() && !faces[j].area_2d().is_empty(), "[Aborting - tried to create unlinked halfblocks from empty pzbs reps.]\n");
							*oi++ = std::shared_ptr<surface>(new surface(faces[i], e));
							*oi++ = std::shared_ptr<surface>(new surface(faces[j], e));
						}
						processed[i] = processed[j] = true;
						break;
					}
				}
			}
			if (!processed[i]) {
				PRINT_BLOCKS("[halfblock is unpaired]\n");
				*oi++ = std::shared_ptr<surface>(new surface(faces[i], e));
			}
		}
		if (g_opts.flags & SBT_VERBOSE_BLOCKS) {
			NOTIFY_MSG( "[parallel matrix follows]\n");
			for (int i = 0; i < 6; ++i) {
				for (int j = 0; j < 6; ++j) {
					NOTIFY_MSG( "%c", are_parallel(i, j) ? '+' : '-');
				}
				NOTIFY_MSG( "\n");
			}
		}
		return true;
	}
	return false; 
}

/*******************************************************************************************************************
 * SECTION THE THIRD: ACTUAL LOGIC
 ******************************************************************************************************************/

typedef CGAL::Envelope_diagram_2<Env_pzbs_surface_traits_3>	envelope_diagram;

std::vector<std::shared_ptr<surface>> convert_to_blocks(std::list<oriented_area> regions, std::shared_ptr<element> & e) {

	std::vector<std::shared_ptr<surface>> results;

	while (!regions.empty()) {
		auto p = regions.begin();
		auto q = p;
		auto curr_closest = regions.end();
		NT closest_distance;
		while (q != regions.end()) {
			if (p != q && 
				p->sense() != q->sense() && 
				(p->height() < q->height() == q->sense()) &&
				oriented_area::are_parallel(*p, *q) &&
				p->area_2d() == q->area_2d())
			{
				if (curr_closest == regions.end() || CGAL::abs(p->height() - q->height()) < closest_distance) {
					curr_closest = q;
					closest_distance = CGAL::abs(p->height() - q->height());
				}
			}
			++q;
		}
		if (curr_closest == regions.end()) {
			results.push_back(std::shared_ptr<surface>(new surface(std::move(*p), e)));
		}
		else {
			std::shared_ptr<surface> a(new surface(std::move(*p), e));
			std::shared_ptr<surface> b(new surface(std::move(*curr_closest), e));
			surface::set_other_sides(a, b);
			results.push_back(a);
			results.push_back(b);
			regions.erase(curr_closest);
		}
		regions.pop_front();
	}

	return results;
}

template <class OutputIterator>
void get_halfblocks_for_this_base(const std::vector<oriented_area> & faces, int base_ix, equality_context * c, OutputIterator oi) {
	PRINT_BLOCKS("[getting halfblocks for:]\n");
	PRINT_BLOCKS_PZBS(faces[base_ix]);
	std::vector<relative_pzbs> relative_reps;
	boost::transform(faces, std::back_inserter(relative_reps), [&c](const oriented_area & rep) { return relative_pzbs(rep, c); });
	PRINT_BLOCKS("[created pair bases]\n");
	boost::for_each(relative_reps, [&relative_reps, base_ix](relative_pzbs & rel) { rel.set_reference(relative_reps[base_ix]); });
	PRINT_BLOCKS("[established initial pairs]\n");

	std::vector<rel_pair> relevances;
	int rel_count = 0;
	std::vector<relative_pzbs>::iterator single_relevant;
	relative_pzbs::relevance_t single_relevance;
	for (auto p = relative_reps.begin(); p != relative_reps.end(); ++p) {
		relevances.push_back(std::make_pair(p, p->get_relevance()));
		if (relevances.back().second != relative_pzbs::IRRELEVANT) {
			PRINT_BLOCKS("[relevance: %s (<%f, %f, %f>)]\n", 
				relevances.back().second == relative_pzbs::PARALLEL_SAME_AREAS ? "area match" :
				relevances.back().second == relative_pzbs::PARALLEL ? "parallel" :
				relevances.back().second == relative_pzbs::NONPARALLEL ? "non-parallel overlap" : "none",
				CGAL::to_double(p->pzbs().orientation().dx()),
				CGAL::to_double(p->pzbs().orientation().dy()),
				CGAL::to_double(p->pzbs().orientation().dz()));
			++rel_count;
			single_relevant = p;
			single_relevance = relevances.back().second;
		}
	}
	PRINT_BLOCKS("[got %u relevances]\n", rel_count);
	if (rel_count == 1) {
		if (single_relevance == relative_pzbs::PARALLEL_SAME_AREAS) {
			*oi++ = single_relevant->relative_to();
			NOTIFY_MSG(".");
			return;
		}
		else if (single_relevance == relative_pzbs::PARALLEL) {
			*oi++ = oriented_area(single_relevant->relative_to(), single_relevant->relative_to().area_2d() * single_relevant->pzbs().area_2d());
			NOTIFY_MSG(".");
			return;
		}
	}
	
	PRINT_BLOCKS(
		"[performing envelope calculation for %u relevant surfaces]\n", 
		std::count_if(relevances.begin(), relevances.end(), [](const rel_pair & rel) { return rel.second != relative_pzbs::IRRELEVANT; }));
	envelope_diagram diagram;
	CGAL::lower_envelope_3(relevances.begin(), relevances.end(), diagram);
	PRINT_BLOCKS("[envelope calculation came back with %u faces]\n", diagram.number_of_faces());
	
	for (auto p = diagram.faces_begin(); p != diagram.faces_end(); ++p) {
		if (!p->is_unbounded()) {
			polygon_2 this_poly;
			auto ccb = p->outer_ccb();
			auto end = ccb;
			CGAL_For_all(ccb, end) {
				if (this_poly.is_empty() ||
					(!c->are_effectively_same(ccb->target()->point(), this_poly.vertex(this_poly.size() - 1)) && !c->are_effectively_same(ccb->target()->point(), *this_poly.vertices_begin()))) {
					this_poly.push_back(ccb->target()->point());
				}
			};
			if (FLAGGED(SBT_VERBOSE_BLOCKS)) {
				NOTIFY_MSG( "[envelope face]\n");
				util::printing::print_polygon(g_opts.notify_func, this_poly);
			}
			if (!util::cgal::polygon_has_no_adjacent_duplicates(this_poly, c)) {
				NOTIFY_MSG("[Bad polygon follows]\n");
				util::printing::print_polygon(g_opts.notify_func, this_poly);
				SBT_ASSERT(false, "[Aborting - an envelope face had adjacent duplicate points.]\n");
			}
			// i don't know why these degenerate faces are showing up, but they are
			if (!geometry_common::cleanup_loop(&this_poly, g_opts.equality_tolerance)) {
				PRINT_BLOCKS("[dropping]\n");
				continue;
			}
			area intr = faces[base_ix].area_2d() * area(std::move(this_poly));
			if (!intr.is_empty()) {
				*oi++ = oriented_area(faces[base_ix], std::move(intr));
			}
		}
	}
	NOTIFY_MSG(".");
}

template <class OutputIterator>
void build_blocks_for(std::shared_ptr<element> e, equality_context * c, OutputIterator oi) {
	NOTIFY_MSG("Building blocks for element %s:", e->source().c_str());

	std::vector<int> still_needs_processing = special_case_finder(e, c).process_and_get_remaining_indices(oi);

	auto as_faces = e->geometry().oriented_faces(c);
	
	if (!still_needs_processing.empty()) {
		std::list<oriented_area> halfblocks;
		std::for_each(still_needs_processing.begin(), still_needs_processing.end(), [&halfblocks, &as_faces, c](int i) {
			get_halfblocks_for_this_base(as_faces, i, c, std::back_inserter(halfblocks));
		});

		PRINT_BLOCKS("[halfblocks follow]\n");
		for (auto p = halfblocks.begin(); p != halfblocks.end(); ++p) {
			PRINT_BLOCKS_PZBS(*p);
		}

		std::vector<std::shared_ptr<surface>> this_element = convert_to_blocks(halfblocks, e);
		NOTIFY_MSG("done. %u surfaces resulted.\n", this_element.size());
		if (FLAGGED(SBT_EXPENSIVE_CHECKS && e->is_fenestration())) {
			boost::for_each(this_element, [&e](std::shared_ptr<surface> & s) {
				if (s->opposite().expired()) {
					ERROR_MSG("Element %s is a fenestration with unpaired halfblocks.\n", e->source_id().c_str());
					abort();
				}
			});
		}
		std::copy(this_element.begin(), this_element.end(), oi);
	}
}

} // namespace

namespace operations {

std::vector<std::shared_ptr<surface>> build_blocks(const std::vector<std::shared_ptr<element>> & elements, std::shared_ptr<equality_context> context) {

	std::vector<std::shared_ptr<surface>> blocked_surfaces;

	for (auto p = elements.begin(); p != elements.end(); ++p) {
		build_blocks_for(*p, context.get(), std::back_inserter(blocked_surfaces));
	}

	return blocked_surfaces;

}

}