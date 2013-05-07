#pragma once

#include "precompiled.h"

#include "geometry_common.h"
#include "nef_polygon_face.h"
#include "nef_polygon_util.h"
#include "polygon_with_holes_2.h"

namespace geometry_2d {

namespace nef_polygons {

class wrapped_nef_polygon {
private:
	std::unique_ptr<nef_polygon_2> wrapped;
	mutable boost::optional<nef_polygon_2::Explorer> current_explorer;

	explicit wrapped_nef_polygon(const nef_polygon_2 & nef)
		: wrapped(new nef_polygon_2(util::clean(nef)))
	{ }

	explicit wrapped_nef_polygon(const face & f);

	std::vector<face> get_faces() const;

public:
	wrapped_nef_polygon() 
		: wrapped(new nef_polygon_2(nef_polygon_2::EMPTY))
	{ }
	
	wrapped_nef_polygon(const wrapped_nef_polygon & src) { *this = src; }
	wrapped_nef_polygon(wrapped_nef_polygon && src) { *this = std::move(src); }

	explicit wrapped_nef_polygon(const polygon_2 & poly)
		: wrapped(new nef_polygon_2(util::create_nef_polygon(poly)))
	{ }

	template <typename PolyRange>
	explicit wrapped_nef_polygon(const PolyRange & polys) 
		: wrapped(new nef_polygon_2()) 
	{
		boost::for_each(polys, [this](const polygon_2 & poly) {
			*wrapped ^= *wrapped_nef_polygon(poly).wrapped;
		});
	}

	explicit wrapped_nef_polygon(const std::vector<std::vector<point_2>> & loops) {
		std::vector<polygon_2> polys;
		for (auto loop = loops.begin(); loop != loops.end(); ++loop) {
			polys.push_back(polygon_2(loop->begin(), loop->end()));
		}
		*this = wrapped_nef_polygon(polys);
	}

	wrapped_nef_polygon & operator = (const wrapped_nef_polygon & src) { 
		if (&src != this) {
			wrapped = std::unique_ptr<nef_polygon_2>(new nef_polygon_2(*src.wrapped));
		}
		return *this; 
	}
	wrapped_nef_polygon & operator = (wrapped_nef_polygon && src) { 
		if (&src != this) { wrapped = std::move(src.wrapped); }
		return *this;
	}
	
	bool								any_points_satisfy_predicate(const std::function<bool(point_2)> & pred) const;
	bbox_2								bbox() const;
	size_t								face_count() const { return get_faces().size(); }
	bool								is_empty() const { return !wrapped || wrapped->is_empty() || get_faces().empty(); }
	std::vector<polygon_2>				to_simple_convex_pieces() const;
	std::string							to_string() const;
	std::vector<polygon_with_holes_2>	to_pwhs() const;
	size_t								vertex_count() const { return wrapped ? util::vertex_count(*wrapped) : 0; }
	NT									outer_regular_area() const;
	wrapped_nef_polygon					update_all(const std::function<point_2(point_2)> & updater) const;

	bool is_valid(const equality_context & c) const;
	bool is_valid(double eps) const { return is_valid(equality_context(eps)); }

	void clear();
	
	wrapped_nef_polygon & operator += (const wrapped_nef_polygon & src);
	wrapped_nef_polygon & operator *= (const wrapped_nef_polygon & src);
	wrapped_nef_polygon & operator -= (const wrapped_nef_polygon & src);
	wrapped_nef_polygon & operator ^= (const wrapped_nef_polygon & src);

	static bool do_intersect(const wrapped_nef_polygon & a, const wrapped_nef_polygon & b);

	friend bool operator == (const wrapped_nef_polygon & lhs, const wrapped_nef_polygon & rhs);
	friend bool operator != (const wrapped_nef_polygon & lhs, const wrapped_nef_polygon & rhs);
	friend bool operator < (const wrapped_nef_polygon & lhs, const wrapped_nef_polygon & rhs);
	friend bool operator > (const wrapped_nef_polygon & lhs, const wrapped_nef_polygon & rhs);
	friend bool operator <= (const wrapped_nef_polygon & lhs, const wrapped_nef_polygon & rhs);
	friend bool operator >= (const wrapped_nef_polygon & lhs, const wrapped_nef_polygon & rhs);
};

inline bool operator == (const wrapped_nef_polygon & lhs, const wrapped_nef_polygon & rhs) {
	return *lhs.wrapped == *rhs.wrapped;
}

inline bool operator != (const wrapped_nef_polygon & lhs, const wrapped_nef_polygon & rhs) {
	return *lhs.wrapped != *rhs.wrapped;
}

inline bool operator < (const wrapped_nef_polygon & lhs, const wrapped_nef_polygon & rhs) {
	return *lhs.wrapped < *rhs.wrapped;
}

inline bool operator > (const wrapped_nef_polygon & lhs, const wrapped_nef_polygon & rhs) {
	return *lhs.wrapped > *rhs.wrapped;
}

inline bool operator <= (const wrapped_nef_polygon & lhs, const wrapped_nef_polygon & rhs) {
	return *lhs.wrapped <= *rhs.wrapped;
}

inline bool operator >= (const wrapped_nef_polygon & lhs, const wrapped_nef_polygon & rhs) {
	return *lhs.wrapped >= *rhs.wrapped;
}

inline wrapped_nef_polygon operator + (
	wrapped_nef_polygon lhs,
	const wrapped_nef_polygon & rhs)
{
	return lhs += rhs;
}

inline wrapped_nef_polygon operator * (
	wrapped_nef_polygon lhs,
	const wrapped_nef_polygon & rhs)
{
	return lhs *= rhs;
}

inline wrapped_nef_polygon operator - (
	wrapped_nef_polygon lhs,
	const wrapped_nef_polygon & rhs)
{
	return lhs -= rhs;
}

} // namespace nef_polygons

} // namespace geometry_2d

typedef geometry_2d::nef_polygons::wrapped_nef_polygon wrapped_nef_polygon;