// Copyright (c) 1997  
// Utrecht University (The Netherlands),
// ETH Zurich (Switzerland),
// INRIA Sophia-Antipolis (France),
// Max-Planck-Institute Saarbruecken (Germany),
// and Tel-Aviv University (Israel).  All rights reserved. 
//
// This file is part of CGAL (www.cgal.org); you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 3 of the License,
// or (at your option) any later version.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL: svn+ssh://scm.gforge.inria.fr/svn/cgal/branches/releases/CGAL-4.1-branch/Intersections_3/include/CGAL/intersection_3.h $
// $Id: intersection_3.h 67093 2012-01-13 11:22:39Z lrineau $
// 
//
// Author(s)     : Geert-Jan Giezeman <geert@cs.uu.nl>


#ifndef CGAL_INTERSECTION_3_H
#define CGAL_INTERSECTION_3_H

#include <CGAL/intersection_3_1.h>
#include <CGAL/Triangle_3_Line_3_do_intersect.h>
#include <CGAL/Triangle_3_Plane_3_do_intersect.h>
#include <CGAL/Triangle_3_Point_3_do_intersect.h> 
#include <CGAL/Triangle_3_Ray_3_do_intersect.h>
#include <CGAL/Triangle_3_Segment_3_do_intersect.h> 
#include <CGAL/Triangle_3_Tetrahedron_3_do_intersect.h> 
#include <CGAL/Triangle_3_Triangle_3_do_intersect.h>

#include <CGAL/internal/Intersections_3/Bbox_3_Bbox_3_do_intersect.h>
#include <CGAL/internal/Intersections_3/Bbox_3_Line_3_do_intersect.h>
#include <CGAL/internal/Intersections_3/Bbox_3_Ray_3_do_intersect.h>
#include <CGAL/internal/Intersections_3/Bbox_3_Segment_3_do_intersect.h>
#include <CGAL/internal/Intersections_3/Bbox_3_Plane_3_do_intersect.h>
#include <CGAL/internal/Intersections_3/Bbox_3_Sphere_3_do_intersect.h>
#include <CGAL/internal/Intersections_3/Bbox_3_Triangle_3_do_intersect.h>

#include <CGAL/internal/Intersections_3/Triangle_3_Line_3_intersection.h>
#include <CGAL/internal/Intersections_3/Triangle_3_Ray_3_intersection.h>
#include <CGAL/internal/Intersections_3/Triangle_3_Segment_3_intersection.h>
#include <CGAL/Triangle_3_Triangle_3_intersection.h>

#endif // CGAL_INTERSECTION_3_H
