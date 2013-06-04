// Copyright (c) 2011 GeometryFactory (France).
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
// $URL$
// $Id$
//
//
// Author(s)     : Sebastien Loriot

#ifndef CGAL_INTERNAL_POLYHEDRON_CONSTNESS_TYPES_H
#define CGAL_INTERNAL_POLYHEDRON_CONSTNESS_TYPES_H

namespace CGAL {
namespace internal_IOP{

template <class Polyhedron,class T>
struct Polyhedron_types;

template <class Polyhedron>
struct Polyhedron_types<Polyhedron,Tag_false>{
  typedef Polyhedron&                                        Polyhedron_ref;
  typedef typename Polyhedron::Halfedge_handle               Halfedge_handle;
  typedef typename Polyhedron::Halfedge_iterator             Halfedge_iterator;
  typedef typename Polyhedron::Facet_iterator                Facet_iterator;
  typedef typename Polyhedron::Facet_handle                  Facet_handle;
  typedef typename Polyhedron::Vertex_handle                 Vertex_handle;
  typedef typename Polyhedron::Vertex                        Vertex;
  typedef typename Polyhedron::Halfedge                      Halfedge;
  typedef typename Polyhedron::Facet                         Facet;
};

template <class Polyhedron>
struct Polyhedron_types<Polyhedron,Tag_true>{
  typedef const Polyhedron&                                  Polyhedron_ref;
  typedef typename Polyhedron::Halfedge_const_handle         Halfedge_handle;
  typedef typename Polyhedron::Halfedge_const_iterator       Halfedge_iterator;
  typedef typename Polyhedron::Facet_const_iterator          Facet_iterator;
  typedef typename Polyhedron::Facet_const_handle            Facet_handle;
  typedef typename Polyhedron::Vertex_const_handle           Vertex_handle;
  typedef const typename Polyhedron::Vertex                  Vertex;
  typedef const typename Polyhedron::Halfedge                Halfedge;
  typedef const typename Polyhedron::Facet                   Facet;
};

} } //namespace CGAL::internal_IOP

#endif //CGAL_INTERNAL_POLYHEDRON_CONSTNESS_TYPES_H
