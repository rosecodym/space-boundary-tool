// Copyright (c) 2005  Stanford University (USA).
// All rights reserved.
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
// $URL: svn+ssh://scm.gforge.inria.fr/svn/cgal/branches/releases/CGAL-4.1-branch/Kinetic_data_structures/include/CGAL/Kinetic/internal/Kernel/Center.h $
// $Id: Center.h 67093 2012-01-13 11:22:39Z lrineau $
// 
//
// Author(s)     : Daniel Russel <drussel@alumni.princeton.edu>

#ifndef CGAL_KINETIC_INTERNAL_CENTER_H
#define CGAL_KINETIC_INTERNAL_CENTER_H
#include <CGAL/Kinetic/basic.h>

namespace CGAL { namespace Kinetic { namespace internal {

template <class K>
struct Center
{
    typedef typename K::Point_3 argument_type;
    typedef typename K::Point_3 result_type;

    const result_type& operator()(const argument_type &p) const
    {
        return p;
    }
    const result_type& operator()(const typename K::Weighted_point_3 &wp) const
    {
        return wp.point();
    }
};

} } } //namespace CGAL::Kinetic::internal
#endif
