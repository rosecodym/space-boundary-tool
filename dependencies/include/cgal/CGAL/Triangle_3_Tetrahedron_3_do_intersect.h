// Copyright (c) 2005  
// Utrecht University (The Netherlands),
// ETH Zurich (Switzerland),
// INRIA Sophia-Antipolis (France),
// Max-Planck-Institute Saarbruecken (Germany),
// and Tel-Aviv University (Israel).  All rights reserved. 
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
// $URL: svn+ssh://scm.gforge.inria.fr/svn/cgal/branches/releases/CGAL-4.1-branch/Intersections_3/include/CGAL/Triangle_3_Tetrahedron_3_do_intersect.h $
// $Id: Triangle_3_Tetrahedron_3_do_intersect.h 70837 2012-07-28 06:21:06Z glisse $
// 
//
// Author(s)     : Nico Kruithof

#ifndef CGAL_TRIANGLE_3_TETRAHEDRON_3_DO_INTERSECT_H
#define CGAL_TRIANGLE_3_TETRAHEDRON_3_DO_INTERSECT_H

#include <CGAL/Triangle_3_Triangle_3_do_intersect.h>

namespace CGAL {

namespace internal {

// This code is not optimized:
template <class K>
typename K::Boolean
do_intersect(const typename K::Triangle_3 &tr,
             const typename K::Tetrahedron_3 &tet,
             const K & k)
{
    typedef typename K::Triangle_3 Triangle;

    CGAL_kernel_precondition( ! k.is_degenerate_3_object() (tr) );
    CGAL_kernel_precondition( ! k.is_degenerate_3_object() (tet) );

    if (do_intersect(tr, Triangle(tet[0], tet[1], tet[2]), k)) return true;
    if (do_intersect(tr, Triangle(tet[0], tet[1], tet[3]), k)) return true;
    if (do_intersect(tr, Triangle(tet[0], tet[2], tet[3]), k)) return true;
    if (do_intersect(tr, Triangle(tet[1], tet[2], tet[3]), k)) return true;

    CGAL_kernel_assertion(k.bounded_side_3_object()(tet, tr[0]) ==
                          k.bounded_side_3_object()(tet, tr[1]));
    CGAL_kernel_assertion(k.bounded_side_3_object()(tet, tr[0]) ==
                          k.bounded_side_3_object()(tet, tr[2]));

    return k.has_on_bounded_side_3_object()(tet, tr[0]);
}


template <class K>
inline
typename K::Boolean
do_intersect(const typename K::Tetrahedron_3 &tet,
	     const typename K::Triangle_3 &tr,
	     const K & k)
{
  return do_intersect(tr, tet, k);
}

} // namespace internal



template <class K>
inline bool do_intersect(const Tetrahedron_3<K> &tet,
			 const Triangle_3<K> &tr)
{
  return typename K::Do_intersect_3()(tr,tet);
}

template <class K>
inline bool do_intersect(const Triangle_3<K> &tr,
			 const Tetrahedron_3<K> &tet)
{
  return typename K::Do_intersect_3()(tr,tet);
}

} //namespace CGAL

#endif // CGAL_TRIANGLE_3_TETRAHEDRON_3_DO_INTERSECT_H
