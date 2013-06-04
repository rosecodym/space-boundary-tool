// Copyright (c) 2004   INRIA Sophia-Antipolis (France).
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
// $URL: svn+ssh://scm.gforge.inria.fr/svn/cgal/branches/releases/CGAL-4.1-branch/Triangulation_3/include/CGAL/Regular_triangulation_filtered_traits_3.h $
// $Id: Regular_triangulation_filtered_traits_3.h 67117 2012-01-13 18:14:48Z lrineau $
//
// Author(s)     : Sylvain Pion

#ifndef CGAL_REGULAR_TRIANGULATION_FILTERED_TRAITS_3_H
#define CGAL_REGULAR_TRIANGULATION_FILTERED_TRAITS_3_H

#ifndef CGAL_NO_DEPRECATED_CODE

#include <CGAL/Regular_triangulation_euclidean_traits_3.h>


namespace CGAL{
 

// The argument is supposed to be a Filtered_kernel like kernel.
template < typename K>
class Regular_triangulation_filtered_traits_3
  : public Regular_triangulation_euclidean_traits_3<K>
{};

	

} //namespace CGAL::internal

#endif //CGAL_NO_DEPRECATED_CODE

#endif // CGAL_REGULAR_TRIANGULATION_FILTERED_TRAITS_3_H
