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
// $URL: svn+ssh://scm.gforge.inria.fr/svn/cgal/branches/releases/CGAL-4.1-branch/Principal_component_analysis/include/CGAL/linear_least_squares_fitting_points_3.h $
// $Id: linear_least_squares_fitting_points_3.h 70936 2012-08-01 13:29:16Z lrineau $
//
// Author(s) : Pierre Alliez and Sylvain Pion and Ankit Gupta

#ifndef CGAL_LINEAR_LEAST_SQUARES_FITTING_POINTS_3_H
#define CGAL_LINEAR_LEAST_SQUARES_FITTING_POINTS_3_H

#include <CGAL/basic.h>
#include <CGAL/Object.h>
#include <CGAL/centroid.h>
#include <CGAL/eigen.h>
#include <CGAL/PCA_util.h>

#include <iterator>

namespace CGAL {

namespace internal {
// fits a plane to a 3D point set
// returns a fitting quality (1 - lambda_min/lambda_max):
//  1 is best (zero variance orthogonally to the fitting line)
//  0 is worst (isotropic case, returns a plane with default direction)
template < typename InputIterator, 
           typename K >
typename K::FT
linear_least_squares_fitting_3(InputIterator first,
                               InputIterator beyond, 
                               typename K::Plane_3& plane, // best fit plane
                               typename K::Point_3& c,     // centroid
                               const typename K::Point_3*, // used for indirection
                               const K& k,                 // kernel
			                         const CGAL::Dimension_tag<0>& tag)
{
  typedef typename K::FT       FT;
  typedef typename K::Point_3  Point;

  // precondition: at least one element in the container.
  CGAL_precondition(first != beyond);

  // compute centroid
  c = centroid(first,beyond,K(),tag);

  // assemble covariance matrix
  FT covariance[6];
  assemble_covariance_matrix_3(first,beyond,covariance,c,k,(Point*) NULL,tag);

  // compute fitting plane
  return fitting_plane_3(covariance,c,plane,k);
} // end fit plane to point set

// fits a line to a 3D point set
// returns a fitting quality (1 - lambda_min/lambda_max):
//  1 is best (zero variance orthogonally to the fitting line)
//  0 is worst (isotropic case, returns a line along x axis)
template < typename InputIterator, 
           typename K >
typename K::FT
linear_least_squares_fitting_3(InputIterator first,
                               InputIterator beyond, 
                               typename K::Line_3& line,  // best fit line
                               typename K::Point_3& c,    // centroid
                               const typename K::Point_3*, // used for indirection
                               const K& k,                // kernel
			                         const CGAL::Dimension_tag<0>& tag)
{
  typedef typename K::FT       FT;
  typedef typename K::Point_3  Point;

  // precondition: at least one element in the container.
  CGAL_precondition(first != beyond);

  // compute centroid
  c = centroid(first,beyond,K(),tag);

  // assemble covariance matrix
  FT covariance[6];
  assemble_covariance_matrix_3(first,beyond,covariance,c,k,(Point*) NULL,tag);

  // compute fitting line
  return fitting_line_3(covariance,c,line,k);
} // end fit line to point set

} // end namespace internal

} //namespace CGAL

#endif // CGAL_LINEAR_LEAST_SQUARES_FITTING_POINTS_3_H
