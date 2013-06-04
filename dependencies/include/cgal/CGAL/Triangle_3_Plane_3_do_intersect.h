// Copyright (c) 2003  INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org); you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 3 of the License,
// or (at your option) any later version.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL: svn+ssh://scm.gforge.inria.fr/svn/cgal/branches/releases/CGAL-4.1-branch/Intersections_3/include/CGAL/Triangle_3_Plane_3_do_intersect.h $
// $Id: Triangle_3_Plane_3_do_intersect.h 67093 2012-01-13 11:22:39Z lrineau $
// 
//
// Author(s)     : Philippe Guigue

#ifndef CGAL_TRIANGLE_3_PLANE_3_DO_INTERSECT_H
#define CGAL_TRIANGLE_3_PLANE_3_DO_INTERSECT_H

namespace CGAL {

namespace internal {

template <class K>
bool do_intersect(const typename K::Triangle_3 &t, 
		  const typename K::Plane_3   &h,
		  const K & k)
{
  
  CGAL_kernel_precondition( ! k.is_degenerate_3_object()(t)) ;
  CGAL_kernel_precondition( ! k.is_degenerate_3_object()(h)) ;

   
  typename K::Construct_vertex_3 vertex_on =
    k.construct_vertex_3_object();
  
  typename K::Oriented_side_3 oriented_side =
    k.oriented_side_3_object();

  

  switch ( oriented_side(h,vertex_on(t,0)) ) {
  case ON_POSITIVE_SIDE:
    return oriented_side(h,vertex_on(t,1)) != ON_POSITIVE_SIDE
      || oriented_side(h,vertex_on(t,2)) != ON_POSITIVE_SIDE;
  case ON_NEGATIVE_SIDE:
    return oriented_side(h,vertex_on(t,1)) != ON_NEGATIVE_SIDE
      || oriented_side(h,vertex_on(t,2)) != ON_NEGATIVE_SIDE ;
  case ON_ORIENTED_BOUNDARY:
    return true;
  default:// should not happen.
    CGAL_kernel_assertion(false);
    return false;
  }
}


template <class K>
inline
bool do_intersect(const typename K::Plane_3   &h,
		  const typename K::Triangle_3 &t, 
		  const K & k)
{
  return do_intersect(t, h, k);
}


} // namespace internal


template <class K>
inline bool do_intersect(const Triangle_3<K> &t, 
			 const Plane_3<K>    &h)
{
  return typename K::Do_intersect_3()(t,h);
}
  
template <class K>
inline bool do_intersect(const Plane_3<K>    &h, 
			 const Triangle_3<K> &t)
{
  return typename K::Do_intersect_3()(t,h);
}

/*
template <class K>
inline bool do_intersect(const Plane_3<K>    &h, 
			 const Triangle_3<K> &t,
			 const K & k)
{
  return do_intersect(t,h,k);
}
*/


} //namespace CGAL

#endif //CGAL_TRIANGLE_3_PLANE_3_DO_INTERSECT_H
