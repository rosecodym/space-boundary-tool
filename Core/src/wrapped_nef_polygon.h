#pragma once

#include "precompiled.h"

#include "geometry_common.h"
#include "nef_polygon_face.h"
#include "nef_polygon_util.h"
#include "nef_polygon_vertex.h"
#include "polygon_with_holes_2.h"

namespace geometry_2d {

namespace nef_polygons {

class wrapped_nef_polygon {
private:
	std::unique_ptr<nef_polygon_2> wrapped;
	bool m_is_axis_aligned;
	mutable boost::optional<nef_polygon_2::Explorer> current_explorer;

	std::vector<face>	get_faces() const;
	std::vector<vertex>	get_vertices() const;

public:
	wrapped_nef_polygon() : wrapped(new nef_polygon_2(nef_polygon_2::EMPTY)), m_is_axis_aligned(true) { }
	
	wrapped_nef_polygon(const wrapped_nef_polygon & src) { *this = src; }
	wrapped_nef_polygon(wrapped_nef_polygon && src) { *this = std::move(src); }

	explicit wrapped_nef_polygon(const polygon_2 & poly)
		: wrapped(new nef_polygon_2(util::create_nef_polygon(poly))),
		m_is_axis_aligned(geometry_common::is_axis_aligned(poly))
	{ }

	template <typename PolyRange>
	explicit wrapped_nef_polygon(const PolyRange & polys) : wrapped(new nef_polygon_2()), m_is_axis_aligned(true) {
		boost::for_each(polys, [this](const polygon_2 & poly) {
			*wrapped ^= *wrapped_nef_polygon(poly).wrapped;
			m_is_axis_aligned &= geometry_common::is_axis_aligned(poly);
		});
	}

	explicit wrapped_nef_polygon(const std::vector<std::vector<point_2>> & loops) {
		*this = wrapped_nef_polygon(loops | boost::adaptors::transformed([](const std::vector<point_2> & loop) { 
			return polygon_2(loop.begin(), loop.end()); 
		}));
	}

	wrapped_nef_polygon & operator = (const wrapped_nef_polygon & src) { 
		if (&src != this) {
			wrapped = std::unique_ptr<nef_polygon_2>(new nef_polygon_2(*src.wrapped));
			m_is_axis_aligned = src.m_is_axis_aligned;
		}
		return *this; 
	}
	wrapped_nef_polygon & operator = (wrapped_nef_polygon && src) { 
		if (&src != this) {
			wrapped = std::move(src.wrapped);
			m_is_axis_aligned = src.m_is_axis_aligned;
		}
		return *this;
	}
	
	bool								any_points_satisfy_predicate(const std::function<bool(point_2)> & pred) const;
	bbox_2								bbox() const;
	bool								is_empty() const { return wrapped && !wrapped->is_empty() && !get_faces().empty(); }
	bool								is_valid(double eps) const;
	void								print_with(const std::function<void(char *)> & func) const;
	std::vector<polygon_2>				to_simple_convex_pieces() const;
	std::vector<polygon_with_holes_2>	to_pwhs() const;

	void								clear();

	wrapped_nef_polygon & operator -= (const wrapped_nef_polygon & src);
	wrapped_nef_polygon & operator ^= (const wrapped_nef_polygon & src);

	static bool do_intersect(const wrapped_nef_polygon & a, const wrapped_nef_polygon & b);

	friend bool operator == (const wrapped_nef_polygon & lhs, const wrapped_nef_polygon & rhs);
	friend bool operator >= (const wrapped_nef_polygon & lhs, const wrapped_nef_polygon & rhs);
	friend wrapped_nef_polygon operator + (const wrapped_nef_polygon & lhs, const wrapped_nef_polygon & rhs);
	friend wrapped_nef_polygon operator - (const wrapped_nef_polygon & lhs, const wrapped_nef_polygon & rhs);
	friend wrapped_nef_polygon operator * (const wrapped_nef_polygon & lhs, const wrapped_nef_polygon & rhs);
};

} // namespace nef_polygons

} // namespace geometry_2d

typedef geometry_2d::nef_polygons::wrapped_nef_polygon wrapped_nef_polygon;