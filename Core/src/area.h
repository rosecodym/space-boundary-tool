#pragma once

#include "precompiled.h"

#include "geometry_common.h"
#include "polygon_with_holes_2.h"
#include "wrapped_nef_polygon.h"

class equality_context;

namespace geometry_2d {

class area {
private:
	polygon_2 simple_rep;
	mutable wrapped_nef_polygon nef_rep;
	mutable bool use_nef;

	void promote() const;
	void ensure_counterclockwise();

	static void promote_both(const area & a, const area & b) { a.promote(); b.promote(); }

public:
	area() : use_nef(false) { }
	explicit area(const polygon_2 & poly) : simple_rep(poly), use_nef(false) { }
	explicit area(polygon_2 && poly) : simple_rep(std::move(poly)), use_nef(false) { }
	explicit area(const std::vector<std::vector<point_2>> & loops);
	explicit area(const std::vector<polygon_2> & loops);
	explicit area(wrapped_nef_polygon && nef) : nef_rep(std::move(nef)), use_nef(true) { }
	
	area(const area & src) { *this = src; }
	area(area && src) { *this = std::move(src); }
	
	area & operator = (const area & src);
	area & operator = (area && src);
	
	bool								any_points_satisfy_predicate(const std::function<bool(point_2)> & pred) const;
	bbox_2								bbox() const { return use_nef ? nef_rep.bbox() : simple_rep.bbox(); }
	bool								is_empty() const { return use_nef ? nef_rep.is_empty() : simple_rep.is_empty(); }
	boost::optional<polygon_2>			outer_bound() const;
	std::vector<polygon_2>				to_simple_convex_pieces() const;
	std::vector<polygon_with_holes_2>	to_pwhs() const;
	size_t								vertex_count() const { return use_nef ? nef_rep.vertex_count() : simple_rep.size(); }
	NT									regular_area() const;

	void clear();

	bool is_valid(double eps) const { return use_nef ? nef_rep.is_valid(eps) : geometry_common::is_valid(simple_rep, eps); }

	std::string to_string() const;
	
	area & operator *= (const area & other);
	area & operator -= (const area & other);
	area & operator ^= (const area & other);

	static bool do_intersect(const area & a, const area & b);

	friend bool operator == (const area & a, const area & b);
	friend bool operator >= (const area & a, const area & b);

	friend area operator + (const area & a, const area & b);
	friend area operator - (const area & a, const area & b);
	friend area operator * (const area & a, const area & b);
};

} // namespace geometry_2d

typedef geometry_2d::area area;