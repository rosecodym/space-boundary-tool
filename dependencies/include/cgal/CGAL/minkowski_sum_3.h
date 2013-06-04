// Copyright (c) 2005-2008 Max-Planck-Institute Saarbruecken (Germany).
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
// $URL: svn+ssh://scm.gforge.inria.fr/svn/cgal/branches/releases/CGAL-4.1-branch/Minkowski_sum_3/include/CGAL/minkowski_sum_3.h $
// $Id: minkowski_sum_3.h 67117 2012-01-13 18:14:48Z lrineau $ 
// 
//
// Author(s)     :  Peter Hachenberger <hachenberger@mpi-sb.mpg.de>
#ifndef CGAL_MINKOWSKI_SUM_3_H
#define CGAL_MINKOWSKI_SUM_3_H

#include <CGAL/convex_decomposition_3.h>
#include <CGAL/Minkowski_sum_3/bipartite_nary_union_sorted_combined.h> 
#include <CGAL/Is_extended_kernel.h>

namespace CGAL {

template<typename Nef_polyhedron_3>
Nef_polyhedron_3 
minkowski_sum_3(Nef_polyhedron_3& N0, Nef_polyhedron_3& N1) 
{
  typedef typename Nef_polyhedron_3::Kernel Kernel;
  typedef typename Is_extended_kernel<Kernel>::value_type Is_extended_kernel;
  if(check_tag(Is_extended_kernel())) {
    std::cerr << "extended kernel is not supported" << std::endl;
    return N0;
  }

  if(N0.volumes_begin()->mark()) {
    std::cerr << "first parameter is an infinite point set" << std::endl;
    return N0;
  }
   
  if(N1.volumes_begin()->mark()) {
    std::cerr << "second parameter is an infinite point set" << std::endl;
    return N1;
  }


  CGAL::convex_decomposition_3<Nef_polyhedron_3>(N0);
  CGAL::convex_decomposition_3<Nef_polyhedron_3>(N1);
  CGAL_assertion(N0.is_valid());
  CGAL_assertion(N1.is_valid());

  Nef_polyhedron_3 result =
    CGAL::bipartite_nary_union_sorted_combined(N0, N1);
  CGAL_assertion(result.is_valid());
  return result;
}

} //namespace CGAL
#endif // CGAL_MINKOWSKI_SUM_3_H
