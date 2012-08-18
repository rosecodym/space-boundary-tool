#pragma once

#include "precompiled.h"

#include "cgal-util.h"
#include "geometry_common.h"
#include "misc-util.h"
#include "polygon_with_holes_2.h"

// HERE BE DRAGONS
// i tried to to a refactor/cleanup, but i ran into what I can only assume is a CGAL bug
// so i can't really touch it

namespace geometry_2d {

class wrapped_nef_polygon {

private:
	std::unique_ptr<nef_polygon_2> wrapped;
	bool m_is_axis_aligned;

	explicit wrapped_nef_polygon(const nef_polygon_2 & nef, bool aligned);

	void snap_to(const wrapped_nef_polygon & other);

	static boost::optional<polygon_with_holes_2> create_pwh_2(const nef_polygon_2::Explorer & e, nef_polygon_2::Explorer::Face_const_handle f);

public:
	wrapped_nef_polygon() : wrapped(new nef_polygon_2(nef_polygon_2::EMPTY)), m_is_axis_aligned(true) { }
	wrapped_nef_polygon(const wrapped_nef_polygon & src) : wrapped(new nef_polygon_2(*src.wrapped)), m_is_axis_aligned(src.m_is_axis_aligned) { }
	wrapped_nef_polygon(wrapped_nef_polygon && src) : wrapped(std::move(src.wrapped)), m_is_axis_aligned(src.m_is_axis_aligned) { }

	explicit wrapped_nef_polygon(const polygon_2 & poly);
	explicit wrapped_nef_polygon(const polygon_with_holes_2 & pwh);

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

	wrapped_nef_polygon & operator -= (const wrapped_nef_polygon & other);
	wrapped_nef_polygon & operator ^= (const wrapped_nef_polygon & other);

	void clear() { wrapped->clear(); m_is_axis_aligned = true; }

	bool is_empty() const;

	bbox_2 bbox() const;

	wrapped_nef_polygon interior() const { return wrapped_nef_polygon(wrapped->interior(), m_is_axis_aligned); }
	
	void print_with(void (*msg_func)(char *)) const;

	std::vector<polygon_with_holes_2> to_pwhs() const;

	polygon_2 outer() const;
	polygon_2 to_single_polygon() const;
	bool is_valid(double eps) const;
	bool is_axis_aligned() const { return m_is_axis_aligned; }
	bool any_points_satisfy_predicate(const std::function<bool(point_2)> & pred) const;

	static bool do_intersect(const wrapped_nef_polygon & a, const wrapped_nef_polygon & b);

	friend bool operator == (const wrapped_nef_polygon & lhs, const wrapped_nef_polygon & rhs);
	friend bool operator < (const wrapped_nef_polygon & lhs, const wrapped_nef_polygon & rhs);
	friend bool operator > (const wrapped_nef_polygon & lhs, const wrapped_nef_polygon & rhs);
	friend bool operator <= (const wrapped_nef_polygon & lhs, const wrapped_nef_polygon & rhs);
	friend bool operator >= (const wrapped_nef_polygon & lhs, const wrapped_nef_polygon & rhs);
	friend wrapped_nef_polygon operator - (const wrapped_nef_polygon & lhs, const wrapped_nef_polygon & rhs);
	friend wrapped_nef_polygon operator * (const wrapped_nef_polygon & lhs, const wrapped_nef_polygon & rhs);

	// deprecated
	template <class OutputIterator>
	void to_simple_polygons(OutputIterator oi) const {
		nef_polygon_2::Explorer e = wrapped->explorer();
		for (auto f = e.faces_begin(); f != e.faces_end(); ++f) {
			if (f->mark()) {
				auto pwh_maybe = create_pwh_2(e, f); // if the face is too small there won't be anything
				if (pwh_maybe) {
					boost::copy(pwh_maybe->to_simple_polygons(), oi);
				}
			}
		}
	}
};

inline bool operator == (const wrapped_nef_polygon & lhs, const wrapped_nef_polygon & rhs) {
	return *lhs.wrapped == *rhs.wrapped;
}

inline bool operator != (const wrapped_nef_polygon & lhs, const wrapped_nef_polygon & rhs) {
	return !(lhs == rhs);
}

inline bool operator < (const wrapped_nef_polygon & lhs, const wrapped_nef_polygon & rhs) {
	return *lhs.wrapped < *rhs.wrapped;
}

inline bool operator <= (const wrapped_nef_polygon & lhs, const wrapped_nef_polygon & rhs) {
	return *lhs.wrapped <= *rhs.wrapped;
}

inline bool operator > (const wrapped_nef_polygon & lhs, const wrapped_nef_polygon & rhs) {
	return *lhs.wrapped > *rhs.wrapped;
}

inline bool operator >= (const wrapped_nef_polygon & lhs, const wrapped_nef_polygon & rhs) {
	return *lhs.wrapped >= *rhs.wrapped;
}

} // namespace geometry_2d

typedef geometry_2d::wrapped_nef_polygon wrapped_nef_polygon;