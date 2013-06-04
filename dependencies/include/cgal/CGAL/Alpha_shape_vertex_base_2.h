// Copyright (c) 1997, 2012  INRIA Sophia-Antipolis (France).
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
// $URL: svn+ssh://scm.gforge.inria.fr/svn/cgal/branches/releases/CGAL-4.1-branch/Alpha_shapes_2/include/CGAL/Alpha_shape_vertex_base_2.h $
// $Id: Alpha_shape_vertex_base_2.h 67216 2012-01-17 15:28:40Z sloriot $
// 
//
// Author(s)     : Tran Kai Frank DA <Frank.Da@sophia.inria.fr>

#ifndef CGAL_ALPHA_SHAPE_VERTEX_BASE_2_H
#define CGAL_ALPHA_SHAPE_VERTEX_BASE_2_H

#include <utility>
#include <CGAL/Triangulation_vertex_base_2.h>
#include <CGAL/internal/Lazy_alpha_nt_2.h>

//-------------------------------------------------------------------
namespace CGAL {
//-------------------------------------------------------------------

template <class Gt, class Vb_ = Default, class ExactAlphaComparisonTag = Tag_false >
class Alpha_shape_vertex_base_2 : public Default::Get<Vb_, Triangulation_vertex_base_2<Gt> >::type
{
  typedef typename Default::Get<Vb_, Triangulation_vertex_base_2<Gt> >::type Vb;
  typedef typename Vb::Triangulation_data_structure  TDS;
public:
  typedef TDS                             Triangulation_data_structure;
  typedef typename TDS::Vertex_handle     Vertex_handle;
  typedef typename TDS::Face_handle       Face_handle;

  typedef typename internal::Alpha_nt_selector_2<Gt,ExactAlphaComparisonTag>::Type_of_alpha Type_of_alpha;
  typedef Type_of_alpha FT;
  typedef std::pair< Type_of_alpha, Type_of_alpha >    Interval2;
  typedef typename Vb::Point                   Point;

  template < typename TDS2 >
  struct Rebind_TDS {
    typedef typename Vb::template Rebind_TDS<TDS2>::Other    Vb2;
    typedef Alpha_shape_vertex_base_2 <Gt,Vb2,ExactAlphaComparisonTag>         Other;
  };
private:
  Interval2 I;

public:
  Alpha_shape_vertex_base_2()
    : Vb() 
    {}
  
  Alpha_shape_vertex_base_2(const Point & p)
    : Vb(p) 
    {}
  
  Alpha_shape_vertex_base_2(const Point & p, Face_handle f)
    : Vb(p, f) 
    {}


public:

  inline Interval2 get_range() 
    {
      return I;
    }

  inline void set_range(Interval2 Inter)
    {  
      I = Inter;
    }

};

//-------------------------------------------------------------------
} //namespace CGAL
//-------------------------------------------------------------------

#endif //ALPHA_SHAPE_VERTEX_BASE_2_H
