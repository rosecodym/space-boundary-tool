// Copyright (c) 2009  INRIA Sophia-Antipolis (France).
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
// $URL: svn+ssh://scm.gforge.inria.fr/svn/cgal/branches/releases/CGAL-4.1-branch/Alpha_shapes_3/include/CGAL/internal/Classification_type.h $
// $Id: Classification_type.h 67117 2012-01-13 18:14:48Z lrineau $
// 
//
// Author(s)     : Sebastien Loriot
//


#ifndef CGAL_INTERNAL_CLASSIFICATION_TYPE_H
#define CGAL_INTERNAL_CLASSIFICATION_TYPE_H

namespace CGAL{
  namespace internal{
    enum Classification_type {EXTERIOR,SINGULAR,REGULAR,INTERIOR};
  }
} 
#endif //CGAL_INTERNAL_CLASSIFICATION_TYPE_H
