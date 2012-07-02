#pragma once

#include "precompiled.h"

#include "area.h"
#include "orientation.h"
#include "polygon_with_holes_3.h"
#include "printing-macros.h"

class equality_context;
class orientation;
class simple_face;

class oriented_area {
private:
	const orientation * o;
	area a;
	NT p;
	bool flipped;

	bbox_3 m_bounding_box;

	oriented_area(const orientation * o, const area & a, const NT & p, bool flipped) : o(o), a(a), p(p), flipped(flipped) { }

	point_3 to_3d(const point_2 & pt) const { return point_3(pt.x(), pt.y(), -p).transform(o->unflattener()); }

	std::vector<point_3> to_3d(const polygon_2 & poly) const {
		std::vector<point_3> res;
		std::transform(poly.vertices_begin(), poly.vertices_end(), std::back_inserter(res), [this](const point_2 & pt) { return to_3d(pt); });
		return res;
	}

	template <typename PointRange>
	std::vector<point_3> to_3d(const PointRange & loop) const {
		return to_3d(polygon_2(loop.begin(), loop.end()));
	}

	typedef orientation orientation_t; // derp derp

public:
	oriented_area(const simple_face & face, equality_context * c);
	oriented_area(nef_halffacet_handle h, equality_context * c);
	oriented_area(const oriented_area & src, const area & new_area)
		: o(src.o), a(new_area), p(src.p), flipped(src.flipped) { }
	oriented_area(const oriented_area & src, area && new_area)
		: o(src.o), a(std::move(new_area)), p(src.p), flipped(src.flipped) { }

	oriented_area(const oriented_area & src) { *this = src; }
	oriented_area(oriented_area && src) { *this = std::move(src); }

	oriented_area & operator = (const oriented_area & src);
	oriented_area & operator = (oriented_area && src);

	const area &			area_2d() const { return a; }
	const orientation &		orientation() const { return *o; }
	NT						height() const { return -p; }
	bool					sense() const { return !flipped; }
	plane_3					backing_plane() const;
	std::vector<ray_3>		drape() const;
	bbox_3					bounding_box() const { return m_bounding_box; }
	oriented_area			reverse() const { return oriented_area(o, a, p, !flipped); }

	bool					any_point_in_halfspace(const plane_3 & defining, equality_context * context_3d) const;

	// THIS METHOD DOESN'T ACCOUNT FOR SENSE INFORMATION
	// for some reason doing that causes level resolution to not work
	// the caller should flip the result themselves if they care
	std::vector<polygon_with_holes_3> to_3d() const;

	oriented_area project_onto_self(const oriented_area & other) const;

	void print() const;

	friend oriented_area operator - (const oriented_area & lhs, const oriented_area & rhs);
	friend oriented_area operator * (const oriented_area & lhs, const oriented_area & rhs);

	static bool are_parallel(const oriented_area & a, const oriented_area & b) { return orientation::are_parallel(*a.o, *b.o); }
	static bool are_perpendicular(const oriented_area & a, const oriented_area & b, double eps = 0.0) { return orientation::are_perpendicular(*a.o, *b.o, eps); }
	static bool areas_match(const oriented_area & a, const oriented_area & b) { return a.a == b.a; }
	static bool same_height(const oriented_area & a, const oriented_area & b) { return a.p == b.p; }
	static boost::optional<NT> could_form_block(const oriented_area & a, const oriented_area & b);

	// DEPRECATED
	plane_3 base_plane() const;
	std::vector<ray_3> drape_rays() const { return drape(); }
	polygon_2 project_onto_self(const polygon_2 & poly, const oriented_area & backing_geometry) const;
	oriented_area(const oriented_area & src, std::shared_ptr<equality_context>) { *this = src; }
	oriented_area(const orientation_t * o, const NT & p, const polygon_2 & geometry, bool sense, std::shared_ptr<equality_context>) {
		*this = oriented_area(o, area(geometry), p, sense); 
	}
	oriented_area(const orientation_t * o, const NT & p, const area & geometry, bool sense) {
		*this = oriented_area(o, geometry, p, sense); 
	}
};

inline oriented_area operator - (const oriented_area & lhs, const oriented_area & rhs) {
	// precondition: orientations and reversedness match
	return oriented_area(lhs.o, lhs.a - rhs.a, lhs.p, lhs.flipped);
}

inline oriented_area operator * (const oriented_area & lhs, const oriented_area & rhs) {
	// precondition: orientations and reversedness match
	return oriented_area(lhs.o, lhs.a * rhs.a, lhs.p, lhs.flipped);
}