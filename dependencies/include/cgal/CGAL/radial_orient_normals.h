// Copyright (c) 2007-08  INRIA Sophia-Antipolis (France).
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
// $URL: svn+ssh://scm.gforge.inria.fr/svn/cgal/branches/releases/CGAL-4.1-branch/Point_set_processing_3/include/CGAL/radial_orient_normals.h $
// $Id: radial_orient_normals.h 67117 2012-01-13 18:14:48Z lrineau $
//
// Author(s) : Laurent Saboret

#ifndef CGAL_RADIAL_ORIENT_NORMALS_H
#define CGAL_RADIAL_ORIENT_NORMALS_H

#include <CGAL/property_map.h>
#include <CGAL/point_set_processing_assertions.h>

#include <math.h>

namespace CGAL {


/// Radial orientation of the [first, beyond) range of points.
/// Normals are oriented towards exterior of the point set.
/// This very fast method is intended to convex objects.
///
/// This method modifies the order of input points so as to pack all oriented points first,
/// and returns an iterator over the first point with an unoriented normal (see erase-remove idiom).
/// For this reason it should not be called on sorted containers.
///
/// @commentheading Precondition: normals must be unit vectors.
///
/// @commentheading Template Parameters:
/// @param ForwardIterator iterator over input points.
/// @param PointPMap is a model of boost::ReadablePropertyMap with a value_type = Point_3<Kernel>.
///        It can be omitted if ForwardIterator value_type is convertible to Point_3<Kernel>.
/// @param NormalPMap is a model of boost::ReadWritePropertyMap with a value_type = Vector_3<Kernel>.
/// @param Kernel Geometric traits class.
///        It can be omitted and deduced automatically from PointPMap value_type.
///
/// @return iterator over the first point with an unoriented normal.

// This variant requires all parameters.
template <typename ForwardIterator,
          typename PointPMap,
          typename NormalPMap,
          typename Kernel
>
ForwardIterator
radial_orient_normals(
    ForwardIterator first,  ///< iterator over the first input point.
    ForwardIterator beyond, ///< past-the-end iterator over the input points.
    PointPMap point_pmap, ///< property map ForwardIterator -> Point_3.
    NormalPMap normal_pmap, ///< property map ForwardIterator -> Vector_3.
    const Kernel& kernel) ///< geometric traits.
{
    CGAL_TRACE("Calls radial_orient_normals()\n");

    // Input points types
    typedef typename std::iterator_traits<ForwardIterator>::value_type Enriched_point;
    typedef typename boost::property_traits<PointPMap>::value_type Point;
    typedef typename boost::property_traits<NormalPMap>::value_type Vector;
    typedef typename Kernel::FT FT;

    // Precondition: at least one element in the container.
    CGAL_point_set_processing_precondition(first != beyond);

    // Find points barycenter.
    // Note: We should use CGAL::centroid() from PCA component.
    //       Unfortunately, it is not compatible with property maps.
    Vector sum = CGAL::NULL_VECTOR;
    int nb_points = 0;
    for (ForwardIterator it = first; it != beyond; it++)
    {
      Point point = get(point_pmap, it);
      sum = sum + (point - CGAL::ORIGIN);
      nb_points++;
    }
    Point barycenter = CGAL::ORIGIN + sum / (FT)nb_points;

    // Iterates over input points and orients normals towards exterior of the point set.
    // Copy points with robust normal orientation to oriented_points[], the others to unoriented_points[].
    std::deque<Enriched_point> oriented_points, unoriented_points;
    for (ForwardIterator it = first; it != beyond; it++)
    {
      Point point = get(point_pmap, it);

      // Radial vector towards exterior of the point set
      Vector vec1 = point - barycenter;

      // Point's normal
      Vector vec2 = normal_pmap[it];

      //         ->               ->
      // Orients vec2 parallel to vec1
      double dot = vec1 * vec2;
      if (dot < 0)
        vec2 = -vec2;

      it->normal() = vec2;

      // Is orientation robust?
      bool oriented = (std::abs(dot) > std::cos(80.*CGAL_PI/180.)); // robust iff angle < 80 degrees
      if (oriented)
        oriented_points.push_back(*it);
      else
        unoriented_points.push_back(*it);
    }

    // Replaces [first, beyond) range by the content of oriented_points[], then unoriented_points[].
    ForwardIterator first_unoriented_point =
      std::copy(oriented_points.begin(), oriented_points.end(), first);
    std::copy(unoriented_points.begin(), unoriented_points.end(), first_unoriented_point);

    CGAL_TRACE("  => %u normals are unoriented\n", unoriented_points.size());
    CGAL_TRACE("End of radial_orient_normals()\n");

    return first_unoriented_point;
}

/// @cond SKIP_IN_MANUAL
// This variant deduces the kernel from the iterator type.
template <typename ForwardIterator,
          typename PointPMap,
          typename NormalPMap
>
ForwardIterator
radial_orient_normals(
    ForwardIterator first,  ///< iterator over the first input point.
    ForwardIterator beyond, ///< past-the-end iterator over the input points.
    PointPMap point_pmap, ///< property map ForwardIterator -> Point_3.
    NormalPMap normal_pmap) ///< property map ForwardIterator -> Vector_3.
{
    typedef typename boost::property_traits<PointPMap>::value_type Point;
    typedef typename Kernel_traits<Point>::Kernel Kernel;
    return radial_orient_normals(
      first,beyond,
      point_pmap, normal_pmap, 
      Kernel());
}
/// @endcond

/// @cond SKIP_IN_MANUAL
// This variant creates a default point property map = Dereference_property_map.
template <typename ForwardIterator,
          typename NormalPMap
>
ForwardIterator
radial_orient_normals(
    ForwardIterator first,  ///< iterator over the first input point.
    ForwardIterator beyond, ///< past-the-end iterator over the input points.
    NormalPMap normal_pmap) ///< property map ForwardIterator -> Vector_3.
{
    return radial_orient_normals(
      first,beyond,
      make_dereference_property_map(first), 
      normal_pmap);
}
/// @endcond


} //namespace CGAL

#endif // CGAL_RADIAL_ORIENT_NORMALS_H
