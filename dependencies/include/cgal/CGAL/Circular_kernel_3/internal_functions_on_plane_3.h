// Copyright (c) 2008  INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
// You can redistribute it and/or modify it under the terms of the GNU
// General Public License as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL: svn+ssh://scm.gforge.inria.fr/svn/cgal/branches/releases/CGAL-4.1-branch/Circular_kernel_3/include/CGAL/Circular_kernel_3/internal_functions_on_plane_3.h $
// $Id: internal_functions_on_plane_3.h 67117 2012-01-13 18:14:48Z lrineau $
//
// Author(s) : Monique Teillaud, Sylvain Pion, Pedro Machado, 
//             Julien Hazebrouck, Damien Leroy

// Partially supported by the IST Programme of the EU as a 
// STREP (FET Open) Project under Contract No  IST-006413 
// (ACS -- Algorithms for Complex Shapes)

#ifndef CGAL_SPHERICAL_KERNEL_PREDICATES_ON_PLANE_3_H
#define CGAL_SPHERICAL_KERNEL_PREDICATES_ON_PLANE_3_H

namespace CGAL {
  namespace SphericalFunctors {

    template < class SK >
    typename SK::Plane_3
    construct_plane_3(const typename SK::Polynomial_1_3 &eq)
    {
      typedef typename SK::Plane_3 Plane_3;
      return Plane_3(eq.a(),eq.b(),eq.c(),eq.d());
    }

  }//SphericalFunctors
}//CGAL

#endif //CGAL_SPHERICAL_KERNEL_PREDICATES_ON_PLANE_3_H
