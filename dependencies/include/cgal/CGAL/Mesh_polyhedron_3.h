// Copyright (c) 2010 INRIA Sophia-Antipolis (France).
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
// $URL: svn+ssh://scm.gforge.inria.fr/svn/cgal/branches/releases/CGAL-4.1-branch/Mesh_3/include/CGAL/Mesh_polyhedron_3.h $
// $Id: Mesh_polyhedron_3.h 67117 2012-01-13 18:14:48Z lrineau $
//
//
// Author(s)     : Stephane Tayeb
//
//******************************************************************************
// File Description : 
//******************************************************************************

#ifndef CGAL_MESH_POLYHEDRON_3_H
#define CGAL_MESH_POLYHEDRON_3_H

#include <CGAL/Polyhedron_3.h>
#include <CGAL/Polyhedron_items_3.h>

#include <set>

namespace CGAL {
namespace Mesh_3 {
  
template <typename Refs, typename Tag, typename Point, typename Patch_id>
class Mesh_polyhedron_vertex : 
public CGAL::HalfedgeDS_vertex_base<Refs, Tag, Point>
{
public:
  typedef std::set<Patch_id> Set_of_indices;
  
private:
  typedef CGAL::HalfedgeDS_vertex_base<Refs, Tag, Point> Pdv_base;
  
  Set_of_indices indices;
public:
  int nb_of_feature_edges;
  
  bool is_corner() const {
    return nb_of_feature_edges > 2;
  }
  
  bool is_feature_vertex() const {
    return nb_of_feature_edges != 0;
  }
  
  void add_incident_patch(const Patch_id i) {
    indices.insert(i);
  }
  
  const Set_of_indices&
  incident_patches_ids_set() const {
    return indices;
  }
  
  Mesh_polyhedron_vertex() : Pdv_base(), nb_of_feature_edges(0) {}
  Mesh_polyhedron_vertex(const Point& p) : Pdv_base(p), nb_of_feature_edges(0) {}
};

template <class Refs, class Tprev, class Tvertex, class Tface>
class Mesh_polyhedron_halfedge : 
public CGAL::HalfedgeDS_halfedge_base<Refs,Tprev,Tvertex,Tface>
{
private:
  bool feature_edge;
public:
  
  Mesh_polyhedron_halfedge() 
  : feature_edge(false) {};
  
  bool is_feature_edge() const {
    return feature_edge;
  }
  
  void set_feature_edge(const bool b) {
    feature_edge = b;
    this->opposite()->feature_edge = b;
  }
};

template <class Refs, class T_, class Pln_, class Patch_id>
class Mesh_polyhedron_face : 
public CGAL::HalfedgeDS_face_base<Refs,T_,Pln_>
{
private:
  Patch_id patch_id_;
public:
  
  Mesh_polyhedron_face() 
  : patch_id_(1) {}
  
  int patch_id() const {
    return patch_id_;
  }
  
  void set_patch_id(const int i) {
    patch_id_ = i;
  }
};

template <typename Patch_id>
class Mesh_polyhedron_items : public CGAL::Polyhedron_items_3 {
public:
  // wrap vertex
  template<class Refs, class Traits> struct Vertex_wrapper
  {
    typedef typename Traits::Point_3 Point;
    typedef Mesh_polyhedron_vertex<Refs,
      CGAL::Tag_true,
      Point,
      Patch_id> Vertex;
  };
  
  // wrap face
  template<class Refs, class Traits> struct Face_wrapper
  {
    typedef Mesh_polyhedron_face<Refs,
      CGAL::Tag_true,
      CGAL::Tag_false,
      Patch_id> Face;
  };
  
  // wrap halfedge
  template<class Refs, class Traits> struct Halfedge_wrapper
  {
    typedef Mesh_polyhedron_halfedge<Refs,
      CGAL::Tag_true,
      CGAL::Tag_true,
      CGAL::Tag_true> Halfedge;
  };
};
  
} // end namespace Mesh_3


template <typename Gt, typename Patch_id=int>
struct Mesh_polyhedron_3
{
  typedef Polyhedron_3<Gt, Mesh_3::Mesh_polyhedron_items<Patch_id> > type;
  typedef type Type;
};

  
} // end namespace CGAL

#endif // CGAL_MESH_POLYHEDRON_3_H
