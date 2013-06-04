// Copyright (c) 2000  Max-Planck-Institute Saarbruecken (Germany).
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
// $URL: svn+ssh://scm.gforge.inria.fr/svn/cgal/branches/releases/CGAL-4.1-branch/Partition_2/include/CGAL/Partition_2/is_degenerate_polygon_2.h $
// $Id: is_degenerate_polygon_2.h 67117 2012-01-13 18:14:48Z lrineau $
// 
//
// Author(s)     : Susan Hert <hert@mpi-sb.mpg.de>

#ifndef CGAL_IS_DEGENERATE_POLYGON_2_H
#define CGAL_IS_DEGENERATE_POLYGON_2_H

namespace CGAL {

// tests if a sequence of points represents a degenerate polygon (i.e.
// one of zero area)
template<class BidirectionalIterator, class Traits>
bool 
is_degenerate_polygon_2(BidirectionalIterator first, 
                        BidirectionalIterator last,
                        const Traits& traits)
{
   if (first == last) return true;

   BidirectionalIterator prev = last;
   prev--;
   BidirectionalIterator curr = first;
   BidirectionalIterator next = first;
   next++;

   // fewer than three vertices
   if (prev == first) return true; 
   if (next == last) return true; 

   typedef typename Traits::Orientation_2                Orientation_2;

   Orientation_2 orientation = traits.orientation_2_object();

   while (curr != last)
   {
     if (orientation(*prev, *curr, *next) != COLLINEAR)
        return false;

     prev++;
     if (prev == last)
        prev = first;
     next++;   
     if (next == last)
       next = first;
     curr++;
   }
   return true;

}

template<class InputIterator>
bool 
is_degenerate_polygon_2(InputIterator first, InputIterator last)
{
   if (first == last) return true;

   typedef typename std::iterator_traits<InputIterator>::value_type Point_2;
   typedef typename Kernel_traits<Point_2>::Kernel  K;
   return is_degenerate_polygon_2(first, last, K());
}

}

#endif // CGAL_IS_DEGENERATE_POLYGON_2_H
