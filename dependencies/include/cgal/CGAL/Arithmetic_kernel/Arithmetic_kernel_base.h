// Copyright (c) 2006-2009 Max-Planck-Institute Saarbruecken (Germany).
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
// $URL: svn+ssh://scm.gforge.inria.fr/svn/cgal/branches/releases/CGAL-4.1-branch/Arithmetic_kernel/include/CGAL/Arithmetic_kernel/Arithmetic_kernel_base.h $
// $Id: Arithmetic_kernel_base.h 67093 2012-01-13 11:22:39Z lrineau $
//
// Author(s)     : Michael Hemmer <hemmer@mpi-inf.mpg.de> 
//
// ============================================================================
//
//    \brief provide base class for Arithmetic_kernel  
//



#ifndef CGAL_ARITHMETIC_KERNEL_ARITHMETIC_KERNEL_BASE_H
#define CGAL_ARITHMETIC_KERNEL_ARITHMETIC_KERNEL_BASE_H

namespace CGAL {
namespace internal{

class Arithmetic_kernel_base{
public:
  typedef CGAL::Null_tag Integer;
  typedef CGAL::Null_tag Rational;
  typedef CGAL::Null_tag Field_with_sqrt;
  typedef CGAL::Null_tag Field_with_kth_root;
  typedef CGAL::Null_tag Field_with_root_of;
  typedef CGAL::Null_tag Bigfloat;
  typedef CGAL::Null_tag Bigfloat_interval;
//  typedef CGAL::Null_tag Exact_float_number;
};

}// namespace internal
} //namespace CGAL

#endif // CGAL_ARITHMETIC_KERNEL_ARITHMETIC_KERNEL_BASE_H
