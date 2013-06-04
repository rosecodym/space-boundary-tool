// Copyright (c) 2007-09  INRIA Sophia-Antipolis (France).
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
// $URL: svn+ssh://scm.gforge.inria.fr/svn/cgal/branches/releases/CGAL-4.1-branch/Point_set_processing_3/include/CGAL/compute_average_spacing.h $
// $Id: compute_average_spacing.h 70936 2012-08-01 13:29:16Z lrineau $
//
// Author(s) : Pierre Alliez and Laurent Saboret

#ifndef CGAL_AVERAGE_SPACING_3_H
#define CGAL_AVERAGE_SPACING_3_H

#include <CGAL/Search_traits_3.h>
#include <CGAL/Orthogonal_k_neighbor_search.h>
#include <CGAL/property_map.h>
#include <CGAL/point_set_processing_assertions.h>

#include <iterator>
#include <list>

namespace CGAL {


// ----------------------------------------------------------------------------
// Private section
// ----------------------------------------------------------------------------
namespace internal {


/// Computes average spacing of one query point from K nearest neighbors.
///
/// @commentheading Precondition: k >= 2.
///
/// @commentheading Template Parameters:
/// @param Kernel Geometric traits class.
/// @param Tree KD-tree.
///
/// @return average spacing (scalar).
template < typename Kernel,
           typename Tree >
typename Kernel::FT
compute_average_spacing(const typename Kernel::Point_3& query, ///< 3D point whose spacing we want to compute
                        Tree& tree,                            ///< KD-tree
                        unsigned int k)                        ///< number of neighbors
{
  // basic geometric types
  typedef typename Kernel::FT FT;
  typedef typename Kernel::Point_3 Point;

  // types for K nearest neighbors search
  typedef Search_traits_3<Kernel> Tree_traits;
  typedef Orthogonal_k_neighbor_search<Tree_traits> Neighbor_search;
  typedef typename Neighbor_search::iterator Search_iterator;

  // performs k + 1 queries (if unique the query point is
  // output first). search may be aborted when k is greater
  // than number of input points
  Neighbor_search search(tree,query,k+1);
  Search_iterator search_iterator = search.begin();
  FT sum_distances = (FT)0.0;
  unsigned int i;
  for(i=0;i<(k+1);i++)
  {
    if(search_iterator == search.end())
      break; // premature ending

    Point p = search_iterator->first;
    sum_distances += std::sqrt(CGAL::squared_distance(query,p));
    search_iterator++;
  }

  // output average spacing
  return sum_distances / (FT)i;
}


} /* namespace internal */


// ----------------------------------------------------------------------------
// Public section
// ----------------------------------------------------------------------------


/// Computes average spacing from k nearest neighbors.
///
/// @commentheading Precondition: k >= 2.
///
/// @commentheading Template Parameters:
/// @param InputIterator iterator over input points.
/// @param PointPMap is a model of boost::ReadablePropertyMap with a value_type = Point_3<Kernel>.
///        It can be omitted if InputIterator value_type is convertible to Point_3<Kernel>.
/// @param Kernel Geometric traits class.
///        It can be omitted and deduced automatically from PointPMap value_type.
///
/// @return average spacing (scalar).

// This variant requires the kernel.
template <typename InputIterator,
          typename PointPMap,
          typename Kernel
>
typename Kernel::FT
compute_average_spacing(
  InputIterator first,  ///< iterator over the first input point.
  InputIterator beyond, ///< past-the-end iterator over the input points.
  PointPMap point_pmap, ///< property map InputIterator -> Point_3
  unsigned int k, ///< number of neighbors.
  const Kernel& /*kernel*/) ///< geometric traits.
{
  // basic geometric types
  typedef typename Kernel::Point_3 Point;

  // types for K nearest neighbors search structure
  typedef typename Kernel::FT FT;
  typedef Search_traits_3<Kernel> Tree_traits;
  typedef Orthogonal_k_neighbor_search<Tree_traits> Neighbor_search;
  typedef typename Neighbor_search::Tree Tree;

  // precondition: at least one element in the container.
  // to fix: should have at least three distinct points
  // but this is costly to check
  CGAL_point_set_processing_precondition(first != beyond);

  // precondition: at least 2 nearest neighbors
  CGAL_point_set_processing_precondition(k >= 2);

  // Instanciate a KD-tree search.
  // Note: We have to convert each input iterator to Point_3.
  std::vector<Point> kd_tree_points; 
  for(InputIterator it = first; it != beyond; it++)
  {
      Point point = get(point_pmap, it);
      kd_tree_points.push_back(point);
  }
  Tree tree(kd_tree_points.begin(), kd_tree_points.end());

  // iterate over input points, compute and output normal
  // vectors (already normalized)
  FT sum_spacings = (FT)0.0;
  unsigned int nb_points = 0;
  for(InputIterator it = first; it != beyond; it++)
  {
    sum_spacings += internal::compute_average_spacing<Kernel,Tree>(get(point_pmap,it),tree,k);
    nb_points++;
  }

  // return average spacing
  return sum_spacings / (FT)nb_points;
}

/// @cond SKIP_IN_MANUAL
// This variant deduces the kernel from the iterator type.
template <typename InputIterator,
          typename PointPMap
>
typename Kernel_traits<typename boost::property_traits<PointPMap>::value_type>::Kernel::FT
compute_average_spacing(
  InputIterator first,    ///< iterator over the first input point.
  InputIterator beyond,   ///< past-the-end iterator over the input points.
  PointPMap point_pmap, ///< property map InputIterator -> Point_3
  unsigned int k) ///< number of neighbors
{
  typedef typename boost::property_traits<PointPMap>::value_type Point;
  typedef typename Kernel_traits<Point>::Kernel Kernel;
  return compute_average_spacing(
    first,beyond,
    point_pmap,
    k,
    Kernel());
}
/// @endcond

/// @cond SKIP_IN_MANUAL
// This variant creates a default point property map = Dereference_property_map.
template < typename InputIterator >
typename Kernel_traits<typename std::iterator_traits<InputIterator>::value_type>::Kernel::FT
compute_average_spacing(
  InputIterator first,    ///< iterator over the first input point.
  InputIterator beyond,   ///< past-the-end iterator over the input points.
  unsigned int k) ///< number of neighbors.
{
  return compute_average_spacing(
    first,beyond,
    make_dereference_property_map(first),
    k);
}
/// @endcond


} //namespace CGAL

#endif // CGAL_AVERAGE_SPACING_3_H

