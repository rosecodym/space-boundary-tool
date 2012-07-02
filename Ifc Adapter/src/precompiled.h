#pragma once

#include <vector>
#include <map>
#include <string>
#include <numeric>
#include <cassert>
#include <cstdio>

#define LEDA_DLL
#define CGAL_LEDA_VERSION 630

#pragma warning (push,1)
#include <CGAL/Cartesian.h>
#include <CGAL/Extended_cartesian.h>
#include <CGAL/CORE_Expr.h>
#include <CGAL/CORE_BigRat.h>
//#include <CGAL/leda_real.h>
//#include <CGAL/leda_rational.h>
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