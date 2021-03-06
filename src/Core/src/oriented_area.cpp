#include "precompiled.h"

#include "oriented_area.h"

#include "area.h"
#include "equality_context.h"
#include "flatten.h"
#include "geometry_common.h"
#include "orientation.h"
#include "simple_face.h"

using namespace boost::adaptors;

namespace {

template <typename PointRange3D>
std::vector<point_2> convert_to_2d(const PointRange3D & loop, const transformation_3 & flatten, equality_context * c) {
	std::vector<point_2> res;
	boost::transform(loop, std::back_inserter(res), [&flatten, c](const point_3 & p) -> point_2 {
		point_3 flattened = flatten(p);
		return c->snap(point_2(flattened.x(), flattened.y()));
	});
	return res;
}

NT calculate_p(const plane_3 & pl) {
	// http://mathworld.wolfram.com/HessianNormalForm.html
	return pl.d() / CGAL::sqrt(pl.a() * pl.a() + pl.b() * pl.b() + pl.c() * pl.c());
}

vector_3 normalized_vector(const direction_3 & d) {
	return d.to_vector() / CGAL::sqrt(d.to_vector().squared_length());
}

} // namespace

oriented_area::oriented_area(const simple_face & f, equality_context * c) {
	std::tie(o, flipped) = c->request_orientation(f.orthogonal_direction());
	flipped = !flipped;

	plane_3 pl(f.average_outer_point(), o->direction());
	p = calculate_p(pl);

	std::vector<std::vector<point_2>> loops;
	loops.push_back(convert_to_2d(f.outer(), o->flattener(), c));
		
	boost::transform(
		f.voids(),
		std::back_inserter(loops),
		[this, c](const std::vector<point_3> & inner) { return convert_to_2d(inner | reversed, o->flattener(), c); });

	a = area(loops);
	
	m_bounding_box = f.outer().front().bbox();
	boost::for_each(f.outer(), [this](const point_3 & p) {
		m_bounding_box = m_bounding_box + p.bbox();
	});
}

oriented_area::oriented_area(nef_halffacet_handle h, equality_context * c) {
	// nef halffacet planes come in flipped because of, i dunno, wizards or something
	plane_3 pl = h->plane().opposite();
	std::tie(o, flipped) = c->request_orientation(pl.orthogonal_direction());
	flipped = !flipped;

	p = calculate_p(flipped ? pl.opposite() : pl);

	m_bounding_box = nef_polyhedron_3::SHalfedge_around_facet_const_circulator(h->facet_cycles_begin())->source()->center_vertex()->point().bbox();

	std::vector<std::vector<point_3>> loops_3d;
	for (auto cycle = h->facet_cycles_begin(); cycle != h->facet_cycles_end(); ++cycle) {
		loops_3d.push_back(std::vector<point_3>());
		nef_polyhedron_3::SHalfedge_around_facet_const_circulator start(cycle);
		nef_polyhedron_3::SHalfedge_around_facet_const_circulator end(cycle);
		CGAL_For_all(start, end) {
			loops_3d.back().push_back(c->snap(start->source()->center_vertex()->point()));
			m_bounding_box = m_bounding_box + loops_3d.back().back().bbox();
		}
	}

	std::vector<std::vector<point_2>> loops_2d;
	boost::transform(loops_3d, std::back_inserter(loops_2d), [this, c](const std::vector<point_3> & loop) {
		return convert_to_2d(loop, o->flattener(), c);
	});
	boost::for_each(loops_2d, [](std::vector<point_2> & loop) {
		if (polygon_2(loop.begin(), loop.end()).is_clockwise_oriented()) {
			boost::reverse(loop);
		}
	});

	a = area(loops_2d);
}

oriented_area::oriented_area(
	const std::vector<point_3> & pts, 
	equality_context * c)
{
	auto pts_info = 
		geometry_common::calculate_plane_and_average_point(pts, *c);
	auto normal = std::get<0>(pts_info).orthogonal_direction();
	std::tie(o, flipped) = c->request_orientation(normal);
	flipped = !flipped;
	p = calculate_p(std::get<0>(pts_info));
	if (flipped) { p = -p; }
	auto bound = convert_to_2d(pts, o->flattener(), c);
	a = area(std::vector<std::vector<point_2>>(1, bound));
	if (!pts.empty()) {
		m_bounding_box = pts.front().bbox();
		for (auto p = pts.begin(); p != pts.end(); ++p) {
			m_bounding_box = m_bounding_box + p->bbox();
		}
	}
}

oriented_area & oriented_area::operator = (const oriented_area & src) {
	if (&src != this) {
		o = src.o;
		a = src.a;
		p = src.p;
		flipped = src.flipped;
	}
	return *this;
}

oriented_area & oriented_area::operator = (oriented_area && src) {
	if (&src != this) {
		o = src.o;
		a = std::move(src.a);
		p = std::move(src.p);
		flipped = src.flipped;
	}
	return *this;
}

plane_3 oriented_area::backing_plane() const {
	vector_3 n = normalized_vector(o->direction());
	return flipped ? plane_3(n.x(), n.y(), n.z(), p).opposite() : plane_3(n.x(), n.y(), n.z(), p);
}

plane_3 oriented_area::parallel_plane_through_origin() const {
	vector_3 n = normalized_vector(o->direction());
	return plane_3(n.x(), n.y(), n.z(), 0.0);
}

std::vector<ray_3> oriented_area::drape() const {
	std::vector<ray_3> res;
	auto outer = a.outer_bound();
	if (outer) {
		boost::transform(to_3d(a.to_pwhs().front().outer()), std::back_inserter(res), [this](const point_3 & pt) {
			return ray_3(pt, flipped ? -o->direction() : o->direction());
		});
		return res;
	}
	else {
		return std::vector<ray_3>();
	}
}

bool oriented_area::any_point_in_halfspace(const plane_3 & defining, equality_context * c3d) const {
	return a.any_points_satisfy_predicate([&defining, c3d, this](const point_2 & p2) { return defining.has_on_positive_side(c3d->snap(to_3d(p2))); });
}

std::vector<polygon_with_holes_3> oriented_area::to_3d(bool flatten_numbers) const {
	std::vector<polygon_with_holes_3> res;
	std::vector<polygon_with_holes_2> pwhs = a.to_pwhs();
	if (flatten_numbers) {
		boost::transform(pwhs, pwhs.begin(), [](const polygon_with_holes_2 & pwh) { return geometry_common::flatten(pwh); });
	}
	for (auto pwh = pwhs.begin(); pwh != pwhs.end(); ++pwh) {
		std::vector<std::vector<point_3>> holes;
		boost::transform(
			pwh->holes(), 
			std::back_inserter(holes), 
			[this](const polygon_2 & hole) { return to_3d(hole); });
		res.push_back(polygon_with_holes_3(to_3d(pwh->outer()), holes));
	}
	return res;
}

oriented_area oriented_area::project_onto_self(const oriented_area & other) const {
	if (other.o == o) {
		return oriented_area(o, other.a, p, other.flipped);
	}
	std::vector<polygon_with_holes_2> projected;
	boost::transform(other.to_3d(), std::back_inserter(projected), [this](const polygon_with_holes_3 pwh) { return pwh.transform(o->flattener()).project_flat(); });
	
	std::vector<polygon_2> all_loops;
	for (auto proj = projected.begin(); proj != projected.end(); ++proj) {
		all_loops.push_back(proj->outer());
		boost::transform(proj->holes(), std::back_inserter(all_loops), [](polygon_2 hole) -> polygon_2 { hole.reverse_orientation(); return hole; });
	}

	return oriented_area(o, area(all_loops), p, all_loops.front().is_clockwise_oriented());
}

bool oriented_area::contains(
	const oriented_area & other, 
	double height_eps) const 
{
	return
		sense() == other.sense() &&
		o == other.o &&
		equality_context::are_equal(height(), other.height(), height_eps) &&
		area_2d() >= other.area_2d();
}

boost::optional<oriented_area> operator - (
	const oriented_area & lhs,
	const oriented_area & rhs)
{
	if (lhs.sense() != rhs.sense() || 
		lhs.o != rhs.o || 
		lhs.height() != rhs.height()) 
	{
		return boost::optional<oriented_area>();
	}
	else { return oriented_area(lhs, lhs.area_2d() - rhs.area_2d()); }
}

boost::optional<oriented_area> operator * (
	const oriented_area & lhs,
	const oriented_area & rhs)
{
#ifndef NDEBUG
	double debug_lhs_height = CGAL::to_double(lhs.height());
	double debug_rhs_height = CGAL::to_double(rhs.height());
	debug_lhs_height = debug_lhs_height;
	debug_rhs_height = debug_rhs_height;
#endif DEBUG
	if (lhs.sense() != rhs.sense() || 
		lhs.o != rhs.o || 
		lhs.height() != rhs.height()) 
	{
		return boost::optional<oriented_area>();
	}
	else { return oriented_area(lhs, lhs.area_2d() * rhs.area_2d()); }
}

boost::optional<NT> oriented_area::could_form_block(const oriented_area & a, const oriented_area & b) {
	if (a.sense() != b.sense() &&
		a.o == b.o &&
		a.height() != b.height() &&
		(a.sense() == a.height() > b.height()) &&
		a.area_2d() == b.area_2d())
	{
		return a.sense() ? a.height() - b.height() : b.height() - a.height();
	}
	else
	{
		return boost::optional<NT>();
	}
}