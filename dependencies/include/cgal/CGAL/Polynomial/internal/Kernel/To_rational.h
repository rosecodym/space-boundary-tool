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
// $URL: svn+ssh://scm.gforge.inria.fr/svn/cgal/branches/releases/CGAL-4.1-branch/Kinetic_data_structures/include/CGAL/Polynomial/internal/Kernel/To_rational.h $
// $Id: To_rational.h 67093 2012-01-13 11:22:39Z lrineau $
// 
//
// Author(s)     : Daniel Russel <drussel@alumni.princeton.edu>

#ifndef CGAL_POLYNOMIAL_INTERNAL_TO_RATIONAL_H
#define CGAL_POLYNOMIAL_INTERNAL_TO_RATIONAL_H

#include <CGAL/Polynomial/basic.h>

#ifdef CGAL_USE_CORE
#include <CGAL/CORE_BigRat.h>
#include <CGAL/CORE_Expr.h>
#endif

namespace CGAL { namespace POLYNOMIAL { namespace internal {

//! Compute the sign after a root.
/*!
  This has specializations for Explicit_roots.
*/
template <class K>
class To_rational
{
public:
  To_rational(){  }

  typedef typename K::FT result_type;
  typedef typename K::Root argument_type;

  template <class T>
  result_type operator()(const T &v) const
  {
    return v.to_rational();
  }

  double operator()(double v) const
  {
    return v;
  }
#ifdef CGAL_USE_CORE
  CORE::BigRat operator()(const CORE::Expr &) const
  {
    return 0;
  }
#endif
};

} } } //namespace CGAL::POLYNOMIAL::internal
#endif
