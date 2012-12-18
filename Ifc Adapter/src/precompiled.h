#pragma once

#pragma warning (push,1)
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
#include <CGAL/CORE_Expr.h>
#include <CGAL/CORE_BigRat.h>
#include <CGAL/Nef_polyhedron_3.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/Polyhedron_incremental_builder_3.h>
#include <CGAL/Interval_skip_list.h>
#include <CGAL/Interval_skip_list_interval.h>
#include <CGAL/Polygon_2.h>

#include <windows.h>
#include <direct.h>
#pragma warning (pop)

#include <cpp_edmi.h>
#include <sdai.h>

#define EPS_MAGIC 0.01