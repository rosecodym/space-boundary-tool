// Copyright (c) 2005  INRIA Sophia-Antipolis (France).
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
// $URL: svn+ssh://scm.gforge.inria.fr/svn/cgal/branches/releases/CGAL-4.1-branch/Surface_mesher/include/CGAL/Weighted_point_with_surface_index_geom_traits.h $
// $Id: Weighted_point_with_surface_index_geom_traits.h 67117 2012-01-13 18:14:48Z lrineau $
// 
//
// Author(s)     : Laurent RINEAU

#ifndef CGAL_WEIGHTED_POINT_WITH_SURFACE_INDEX_GEOM_TRAITS_H
#define CGAL_WEIGHTED_POINT_WITH_SURFACE_INDEX_GEOM_TRAITS_H

#include <CGAL/Weighted_point_with_surface_index.h>

namespace CGAL {

template <class GT>
class Weighted_point_with_surface_index_geom_traits : public GT
{
  typedef typename GT::Point_3 Old_point_3;

public:
  typedef Weighted_point_with_surface_index<Old_point_3> Weighted_point_3;
  typedef Weighted_point_with_surface_index<Old_point_3> Point_3;

};  // end Weighted_point_with_surface_index_geom_traits

} // end namespace CGAL

#endif // CGAL_WEIGHTED_POINT_WITH_SURFACE_INDEX_GEOM_TRAITS_H
