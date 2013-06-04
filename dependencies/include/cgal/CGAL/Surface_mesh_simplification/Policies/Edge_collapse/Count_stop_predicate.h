// Copyright (c) 2006  GeometryFactory (France). All rights reserved.
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
// $URL: svn+ssh://scm.gforge.inria.fr/svn/cgal/branches/releases/CGAL-4.1-branch/Surface_mesh_simplification/include/CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Count_stop_predicate.h $
// $Id: Count_stop_predicate.h 67117 2012-01-13 18:14:48Z lrineau $
//
// Author(s)     : Fernando Cacciola <fernando.cacciola@geometryfactory.com>
//
#ifndef CGAL_SURFACE_MESH_SIMPLIFICATION_POLICIES_EDGE_COLLAPSE_COUNT_STOP_PREDICATE_H
#define CGAL_SURFACE_MESH_SIMPLIFICATION_POLICIES_EDGE_COLLAPSE_COUNT_STOP_PREDICATE_H 1

#include <CGAL/Surface_mesh_simplification/Detail/Common.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Edge_profile.h>

namespace CGAL {

namespace Surface_mesh_simplification
{

//*******************************************************************************************************************
//                                -= stopping condition predicate =-
//
// Determines whether the simplification has finished.
// The arguments are (current_cost,vertex,vertex,is_edge,initial_pair_count,current_pair_count,surface) and the result is bool
//
//*******************************************************************************************************************

// 
// Stops when the number of edges left falls below a given number.
//
template<class ECM_>    
class Count_stop_predicate
{
public:

  typedef ECM_ ECM ;

  typedef Edge_profile<ECM> Profile ;
  
private :

  typedef typename halfedge_graph_traits<ECM>::Point Point ;
  typedef typename Kernel_traits<Point>::Kernel      Kernel ;

public :
  
  typedef typename boost::graph_traits<ECM>::edge_descriptor edge_descriptor ;
  typedef typename boost::graph_traits<ECM>::edges_size_type size_type ;
  
  typedef typename Kernel::FT FT ;

public :
  
  Count_stop_predicate( size_type aThres ) : mThres(aThres) {}
  
  bool operator()( FT const&      // aCurrentCost
                 , Profile const& //aEdgeProfile
		 , size_type      //aInitialCount
                 , size_type      aCurrentCount
                 ) const 
  {
    return aCurrentCount < mThres ;
  }
  
private:
  
  size_type mThres ;
};    

} // namespace Surface_mesh_simplification

} //namespace CGAL

#endif // CGAL_SURFACE_MESH_SIMPLIFICATION_POLICIES_EDGE_COLLAPSE_COUNT_STOP_PREDICATE_H //
// EOF //
 
