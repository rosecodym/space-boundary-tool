#pragma once

#define CGAL_NO_AUTOLINK_GMP
#define CGAL_NO_AUTOLINK_MPFR
#define BOOST_RESULT_OF_USE_DECLTYPE

#pragma warning (disable:4018) // signed/unsigned mismatch
#pragma warning (disable:4127) // conditional expression is constant - for printing macros
#pragma warning (disable:4512) // assignment operator could not be generated

#pragma warning (push,1)
#pragma warning (disable:4701) // potential use of uninitialized variable
#pragma warning (disable:4702) // unreachable code
#pragma warning (disable:4756) // overflow in constant arithmetic
#include <vector>
#include <list>
#include <map>
#include <deque>
#include <queue>
#include <string>
#include <algorithm>
#include <iostream>
#include <memory>
#include <exception>
#include <cassert>
#include <cstdlib>
#include <cmath>
#include <cstdio>
#include <ctime>

#include <windows.h>
#include <eh.h>

#include <boost/format.hpp>
#include <boost/multi_array.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>

#include <CGAL/Cartesian.h>
#include <CGAL/Extended_cartesian.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/Polyhedron_incremental_builder_3.h>
#include <CGAL/Object.h>
#include <CGAL/enum.h>
#include <CGAL/Arr_segment_traits_2.h>
#include <CGAL/envelope_3.h>
#include <CGAL/CORE_Expr.h>
#include <CGAL/Interval_skip_list.h>
#include <CGAL/Interval_skip_list_interval.h>
#include <CGAL/Nef_polyhedron_2.h>
#include <CGAL/Nef_polyhedron_3.h>
#pragma warning (pop)

typedef CORE::Expr						NT;

typedef CGAL::Simple_cartesian<NT>		K;
typedef CGAL::Point_2<K>				point_2;
typedef CGAL::Point_3<K>				point_3;
typedef CGAL::Direction_3<K>			direction_3;
typedef CGAL::Vector_2<K>				vector_2;
typedef CGAL::Vector_3<K>				vector_3;
typedef CGAL::Segment_2<K>				segment_2;
typedef CGAL::Segment_3<K>				segment_3;
typedef CGAL::Ray_3<K>					ray_3;
typedef CGAL::Line_2<K>					line_2;
typedef CGAL::Line_3<K>					line_3;
typedef CGAL::Plane_3<K>				plane_3;
typedef CGAL::Aff_transformation_2<K>	transformation_2;
typedef CGAL::Aff_transformation_3<K>	transformation_3;
typedef CGAL::Polygon_2<K>				polygon_2;
typedef CGAL::Polyhedron_3<K>			polygon_3;
typedef CGAL::Polyhedron_3<K>			polyhedron_3;
typedef CGAL::Nef_polyhedron_3<K>		nef_polyhedron_3;
typedef CGAL::Bbox_2					bbox_2;
typedef CGAL::Bbox_3					bbox_3;

typedef CGAL::Simple_cartesian<double>	iK;
typedef CGAL::Point_3<iK>				ipoint_3;
typedef CGAL::Line_3<iK>				iline_3;

typedef nef_polyhedron_3::Volume_const_iterator		nef_volume_iterator;
typedef nef_polyhedron_3::Volume_const_handle		nef_volume_handle;
typedef nef_polyhedron_3::Vertex_const_handle		nef_vertex_handle;
typedef nef_polyhedron_3::Halfedge_const_handle		nef_halfedge_handle;
typedef nef_polyhedron_3::Halffacet_const_handle	nef_halffacet_handle;
typedef nef_polyhedron_3::SHalfedge_const_handle	nef_shalfedge_handle;
typedef nef_polyhedron_3::SHalfloop_const_handle	nef_shalfloop_handle;
typedef nef_polyhedron_3::SFace_const_handle		nef_sface_handle;

typedef CGAL::Extended_cartesian<NT>	eK;
typedef eK::Point_2						epoint_2;
typedef eK::Segment_2					esegment_2;
typedef eK::Vector_2					evector_2;
typedef eK::Line_2						eline_2;
typedef CGAL::Nef_polyhedron_2<eK>		nef_polygon_2;
typedef eK::Standard_point_2			espoint_2;
typedef eK::Standard_segment_2			essegment_2;
typedef CGAL::Polygon_2<eK>				epolygon_2;