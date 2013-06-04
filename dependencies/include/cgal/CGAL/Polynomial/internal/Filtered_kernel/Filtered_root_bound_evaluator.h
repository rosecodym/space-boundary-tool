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
// $URL: svn+ssh://scm.gforge.inria.fr/svn/cgal/branches/releases/CGAL-4.1-branch/Kinetic_data_structures/include/CGAL/Polynomial/internal/Filtered_kernel/Filtered_root_bound_evaluator.h $
// $Id: Filtered_root_bound_evaluator.h 67093 2012-01-13 11:22:39Z lrineau $
// 
//
// Author(s)     : Daniel Russel <drussel@alumni.princeton.edu>

#ifndef CGAL_POLYNOMIAL_FILTERED_ROOT_BOUND_EVALUATOR_H
#define CGAL_POLYNOMIAL_FILTERED_ROOT_BOUND_EVALUATOR_H

#include <CGAL/Polynomial/basic.h>

namespace CGAL { namespace POLYNOMIAL { namespace internal {

template<class Kernel, class M_t = CGAL::Field_tag>
class Filtered_root_bound_evaluator
{
    public:
        Filtered_root_bound_evaluator(bool pow,
            const Kernel k): rb_(k.interval_kernel_object().root_bound_object(pow))  {}
        typedef double result_type;
        typedef typename Kernel::Function argument_type;

        result_type operator()(const argument_type& p) const
        {
            Interval_arithmetic_guard iag;
            return rb_(p.interval_function()).sup();
        }
    protected:
        typename Kernel::Interval_kernel::Root_bound rb_;
};

} } } //namespace CGAL::POLYNOMIAL::internal
#endif                                            // CGAL_POLYNOMIAL_ROOT_BOUND_EVALUATOR_H
