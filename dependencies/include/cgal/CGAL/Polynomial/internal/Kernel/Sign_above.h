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
// $URL: svn+ssh://scm.gforge.inria.fr/svn/cgal/branches/releases/CGAL-4.1-branch/Kinetic_data_structures/include/CGAL/Polynomial/internal/Kernel/Sign_above.h $
// $Id: Sign_above.h 67093 2012-01-13 11:22:39Z lrineau $
// 
//
// Author(s)     : Daniel Russel <drussel@alumni.princeton.edu>

#ifndef CGAL_POLYNOMIAL_INTERNAL_AFTER_AT_ROOT_H
#define CGAL_POLYNOMIAL_INTERNAL_AFTER_AT_ROOT_H

#include <CGAL/Polynomial/basic.h>
//#include <CGAL/Polynomial/internal/Explicit_root.h>
#include <CGAL/Polynomial/internal/Rational/Evaluate_polynomial.h>
#include <CGAL/Polynomial/internal/Rational/Sign_above_rational.h>

namespace CGAL { namespace POLYNOMIAL { namespace internal {

//! Compute the sign after a root.
/*!
  This has specializations for Explicit_roots.
*/
template <class RNT, class K>
class Sign_above: Sign_above_rational<K>
{
  typedef typename K::Function Poly;
  typedef Sign_above_rational<K> P;
public:
  Sign_above( K k):P(k) {
  }
  Sign_above(){}

  using P::operator();

  typedef CGAL::Sign result_type;
  typedef Poly first_argument_type;
  typedef typename K::Root second_argument_type;
  typename P::result_type operator()(const first_argument_type &f,
				     const second_argument_type &v) const
  {
    CGAL_Polynomial_expensive_precondition(k_.sign_at_root_object(p_)(v)==CGAL::ZERO);
    return eval(f, v);
  }

protected:

  template <class R>
  CGAL::Sign eval(const Poly &p, const R &r) const
  {
    double ub= CGAL::to_interval(r).second+.00001;
    if (ub== CGAL::to_interval(r).second) ub= ub*2;
    typename K::Root_stack rc=  k_.root_stack_object(p, r, ub);
    while (true) {
      if (rc.empty()) {
	return k_.sign_at_object()(p,typename Poly::NT(ub));
      } else if (rc.top() != r) {
	typename K::Root rr=rc.top();
	typename K::Sign_between_roots sb= k_.sign_between_roots_object();
	return sb(p, r, rr);
      } else {
	rc.pop();
      }
    }
  }
  K k_;
};


} } } //namespace CGAL::POLYNOMIAL::internal
#endif
