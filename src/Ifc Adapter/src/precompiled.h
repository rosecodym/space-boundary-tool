#pragma once

#define CGAL_LEDA_VERSION 630
#define LEDA_DLL

#pragma warning (push,1)
#pragma warning (disable:4005) // macro redefinition
#pragma warning (disable:4293) // shift count negative or too big
#pragma warning (disable:4701) // potentially uninitialized local
#pragma warning (disable:4702) // unreachable code
#pragma warning (disable:4756) // overflow in constant arithmetic
#pragma warning (disable:4800) // forcing int to bool
#include <vector>
#include <map>
#include <string>
#include <numeric>
#include <exception>
#include <cassert>
#include <cstdio>

#include <boost/range/algorithm.hpp>
#include <boost/format.hpp>

#include <CGAL/Cartesian.h>
#include <CGAL/Extended_cartesian.h>
#include <CGAL/leda_real.h>
#include <CGAL/leda_rational.h>
#include <CGAL/Nef_polyhedron_3.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/Polyhedron_incremental_builder_3.h>
#include <CGAL/Interval_skip_list.h>
#include <CGAL/Interval_skip_list_interval.h>
#include <CGAL/Polygon_2.h>

#include <windows.h>
#include <direct.h>
#pragma warning (pop)

typedef CGAL::Simple_cartesian<leda_real>	K;
typedef K::FT								NT;
typedef CGAL::Point_2<K>					point_2;
typedef CGAL::Point_3<K>					point_3;
typedef CGAL::Direction_3<K>				direction_3;
typedef CGAL::Vector_3<K>					vector_3;
typedef CGAL::Ray_3<K>						ray_3;
typedef CGAL::Plane_3<K>					plane_3;
typedef CGAL::Aff_transformation_2<K>		transformation_2;
typedef CGAL::Aff_transformation_3<K>		transformation_3;
typedef CGAL::Polygon_2<K>					polygon_2;
typedef CGAL::Polyhedron_3<K>				polyhedron_3;
typedef CGAL::Nef_polyhedron_3<K>			nef_polyhedron_3;

typedef CGAL::Simple_cartesian<double>		iK;
typedef CGAL::Point_2<iK>					ipoint_2;
typedef CGAL::Point_3<iK>					ipoint_3;
typedef CGAL::Direction_3<iK>				idirection_3;
typedef CGAL::Plane_3<iK>					iplane_3;
typedef CGAL::Aff_transformation_2<iK>		itransformation_2;
typedef CGAL::Aff_transformation_3<iK>		itransformation_3;
typedef CGAL::Polygon_2<iK>					ipolygon_2;

#define EPS_MAGIC 0.01