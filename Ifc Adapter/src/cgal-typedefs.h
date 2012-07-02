#pragma once

#include "precompiled.h"

typedef CGAL::Simple_cartesian<CORE::Expr>		K;
typedef K::FT									NT;
typedef CGAL::Point_2<K>						point_2;
typedef CGAL::Point_3<K>						point_3;
typedef CGAL::Direction_3<K>					direction_3;
typedef CGAL::Vector_3<K>						vector_3;
typedef CGAL::Ray_3<K>							ray_3;
typedef CGAL::Plane_3<K>						plane_3;
typedef CGAL::Aff_transformation_2<K>			transformation_2;
typedef CGAL::Aff_transformation_3<K>			transformation_3;

typedef CGAL::Polygon_2<K>						polygon_2;

typedef K										q_K;
typedef q_K::FT									q_NT;
typedef CGAL::Point_3<q_K>						q_point_3;
typedef CGAL::Direction_3<q_K>					q_direction_3;
typedef CGAL::Vector_3<q_K>						q_vector_3;
typedef CGAL::Aff_transformation_3<q_K>			q_transformation_3;
typedef CGAL::Nef_polyhedron_3<q_K>				q_nef_polyhedron_3;
typedef CGAL::Polyhedron_3<q_K>					q_polyhedron_3;