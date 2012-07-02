#pragma once

#include "precompiled.h"

typedef CGAL::Extended_cartesian<CORE::BigRat> K;
//typedef CGAL::Exact_predicates_exact_constructions_kernel K;
//typedef CGAL::Extended_homogeneous<CORE::BigInt> K;

typedef K::FT							NT;
typedef CGAL::Polyhedron_3<K>			polyhedron_3;
typedef CGAL::Nef_polyhedron_3<K>		nef_polyhedron_3;
typedef nef_polyhedron_3::Plane_3		plane_3;
typedef K::Line_3						line_3;
typedef K::Ray_3						ray_3;
typedef K::Vector_3						vector_3;
typedef K::Point_3						point_3;
typedef K::Aff_transformation_3			transformation_3;

typedef nef_polyhedron_3::Halffacet_const_handle nef_halffacet_handle;