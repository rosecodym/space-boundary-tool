// Copyright (c) 2009   INRIA Sophia-Antipolis (France).
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
// $URL: svn+ssh://scm.gforge.inria.fr/svn/cgal/branches/releases/CGAL-4.1-branch/Periodic_3_triangulation_3/include/CGAL/periodic_3_triangulation_3_io.h $
// $Id: periodic_3_triangulation_3_io.h 67117 2012-01-13 18:14:48Z lrineau $
// 
//
// Author(s)     : Manuel Caroli <Manuel.Caroli@sophia.inria.fr>

#ifndef CGAL_PERIODIC_3_TRIANGULATION_3_IO_H
#define CGAL_PERIODIC_3_TRIANGULATION_3_IO_H

template <class Stream, class Triangulation>
Stream &write_triangulation_to_off(Stream &out, Triangulation &t) {
  typedef typename Triangulation::Point Point;
  typedef typename Triangulation::Iso_cuboid Iso_cuboid;
  int number_of_cells = t.tds().number_of_cells();
  out << "OFF "
      << "\n" << 4*number_of_cells + 8
      << " "  << 4*number_of_cells + 6
      << " "  << 0
      << std::endl;

  Iso_cuboid cb = t.domain();

  out << cb.xmin() << " " << cb.ymin() << " " << cb.zmax() << std::endl;
  out << cb.xmax() << " " << cb.ymin() << " " << cb.zmax() << std::endl;
  out << cb.xmin() << " " << cb.ymax() << " " << cb.zmax() << std::endl;
  out << cb.xmax() << " " << cb.ymax() << " " << cb.zmax() << std::endl;
  out << cb.xmin() << " " << cb.ymax() << " " << cb.zmin() << std::endl;
  out << cb.xmax() << " " << cb.ymax() << " " << cb.zmin() << std::endl;
  out << cb.xmin() << " " << cb.ymin() << " " << cb.zmin() << std::endl;
  out << cb.xmax() << " " << cb.ymin() << " " << cb.zmin() << std::endl;

  if (t.number_of_sheets() == CGAL::make_array(1,1,1)) {
    for (typename Triangulation::Cell_iterator it = t.cells_begin();
	 it != t.cells_end(); it++) {
      for (int i=0; i<4; i++) {
	Point p = t.point(t.periodic_point(it,i));
	out << p.x() << " " 
	    << p.y() << " " 
	    << p.z() << std::endl;
      }
    }
  } else {
    for (typename Triangulation::Cell_iterator it = t.cells_begin();
	 it != t.cells_end(); it++) {
      for (int i=0; i<4; i++) {
	typename Triangulation::Vertex_handle vh; 
	typename Triangulation::Offset off;
	t.get_vertex(it, i, vh, off);
	Point p = t.point(t.periodic_point(it, i));
  
	out << p.x() << " " 
	    << p.y() << " " 
	    << p.z() << std::endl;
      }
    }
  }
  out << "4 0 1 3 2" << std::endl;
  out << "4 2 3 5 4" << std::endl;
  out << "4 4 5 7 6" << std::endl;
  out << "4 6 7 1 0" << std::endl;
  out << "4 1 7 5 3" << std::endl;
  out << "4 6 0 2 4" << std::endl;

  for (int i=0; i<number_of_cells; i++) {
    out << "3 " << i*4  +8 << " " << i*4+1+8 << " " << i*4+2+8 << std::endl;
    out << "3 " << i*4  +8 << " " << i*4+1+8 << " " << i*4+3+8 << std::endl;
    out << "3 " << i*4  +8 << " " << i*4+2+8 << " " << i*4+3+8 << std::endl;
    out << "3 " << i*4+1+8 << " " << i*4+2+8 << " " << i*4+3+8 << std::endl;
  }

  return out;
}

template<class Stream, class Triangulation, class Cell_iterator>
Stream &write_cells_to_off(Stream &out, Triangulation &t, int number_of_cells,
			   Cell_iterator cit, Cell_iterator cells_end) {
  typedef typename Triangulation::Point Point;
  out << "OFF "
      << "\n" << 4*number_of_cells
      << " "  << 4*number_of_cells
      << " "  << 0
      << std::endl;

  while (cit != cells_end) {
    for (int i=0; i<4; i++) {
      Point p = t.get_point(*cit,i);
      out << p.x() << " " << p.y() << " " << p.z() << std::endl;
    }
    ++cit;
  }

  for (int i=0; i<number_of_cells; i++) {
    out << "3 " << i*4 << " " << i*4+1 << " " << i*4+2 << std::endl;
    out << "3 " << i*4 << " " << i*4+1 << " " << i*4+3 << std::endl;
    out << "3 " << i*4 << " " << i*4+2 << " " << i*4+3 << std::endl;
    out << "3 " << i*4+1 << " " << i*4+2 << " " << i*4+3 << std::endl;
  }

  return out;
}

#if 0 
//TODO: rewrite this to wrap the stream coming from draw_dual
template <class Stream>
Stream& draw_dual_to_off(Stream &os) {
  os << "OFF " << "\n" 
     << 2*number_of_facets() << " " 
     << number_of_facets() << " 0" << std::endl;
    for (Facet_iterator fit = facets_begin(), end = facets_end();
	 fit != end; ++fit) {
      if (!is_canonical(*fit)) continue;
	std::pair<Segment,Offset> pso = dual(*fit);
      os << pso.first.source() << std::endl
	 << pso.first.target() - pso.second<< std::endl;
    }
  CGAL_triangulation_assertion( i==number_of_facets());
  for(unsigned int i=0 ; i < number_of_facets() ; i++) {
    os << "2 " << i*2 << " " << i*2+1 << std::endl;
  }
  return os;
}
#endif

#endif //CGAL_PERIODIC_3_TRIANGULATION_3_IO_H
