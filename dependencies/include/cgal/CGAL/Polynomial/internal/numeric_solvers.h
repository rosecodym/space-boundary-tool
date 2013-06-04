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
// $URL: svn+ssh://scm.gforge.inria.fr/svn/cgal/branches/releases/CGAL-4.1-branch/Kinetic_data_structures/include/CGAL/Polynomial/internal/numeric_solvers.h $
// $Id: numeric_solvers.h 67093 2012-01-13 11:22:39Z lrineau $
// 
//
// Author(s)     : Daniel Russel <drussel@alumni.princeton.edu>

#ifndef CGAL_POLYNOMIAL_INTERNAL_NUMERIC_SOLVERS_H
#define CGAL_POLYNOMIAL_INTERNAL_NUMERIC_SOLVERS_H
#include <CGAL/Polynomial/basic.h>
#include <vector>

namespace CGAL { namespace POLYNOMIAL { namespace internal {

// JAMA
CGAL_EXPORT
void jama_polynomial_compute_roots(const double *begin, const double *end,
double lb, double ub, std::vector<double> &roots);

CGAL_EXPORT
void jama_polynomial_compute_cleaned_roots(const double *begin, const double *end,
double lb, double ub, std::vector<double> &roots);

// GSL
CGAL_EXPORT
void gsl_polynomial_compute_roots(const double *begin, const double *end,
double lb, double ub, std::vector<double> &roots);

CGAL_EXPORT
void gsl_polynomial_compute_cleaned_roots(const double *begin, const double *end,
double lb, double ub, std::vector<double> &roots);

// Turkowski
CGAL_EXPORT
void Turkowski_polynomial_compute_roots(const double *begin, const double *end,
double lb, double ub, std::vector<double> &roots);

CGAL_EXPORT
void Turkowski_polynomial_compute_cleaned_roots(const double *begin, const double *end,
double lb, double ub, std::vector<double> &roots);

/*struct GSL_numeric_solver {
  void operator()(const double *begin, const double *end,
          double lb, double ub,
          std::vector<double> &roots) const {
    gsl_polynomial_compute_roots(begin, end, lb, ub, roots);
  }
};

struct GSL_cleaned_numeric_solver {
  void operator()(const double *begin, const double *end,
          double lb, double ub,
std::vector<double> &roots) const {
gsl_polynomial_compute_cleaned_roots(begin, end, lb, ub, roots);
}
};*/

struct JAMA_numeric_solver
{
    void operator()(const double *begin, const double *end,
        double lb, double ub,
        std::vector<double> &roots) const
    {
        jama_polynomial_compute_roots(begin, end, lb, ub, roots);
    }
};

struct JAMA_cleaned_numeric_solver
{
    void operator()(const double *begin, const double *end,
        double lb, double ub,
        std::vector<double> &roots) const
    {
        jama_polynomial_compute_cleaned_roots(begin, end, lb, ub, roots);
    }
};

struct Turkowski_numeric_solver
{
    void operator()(const double *begin, const double *end,
        double lb, double ub,
        std::vector<double> &roots) const
    {
        Turkowski_polynomial_compute_roots(begin, end, lb, ub, roots);
    }
};

struct Turkowski_cleaned_numeric_solver
{
    void operator()(const double *begin, const double *end,
        double lb, double ub,
        std::vector<double> &roots) const
    {
        Turkowski_polynomial_compute_cleaned_roots(begin, end, lb, ub, roots);
    }
};

} } } //namespace CGAL::POLYNOMIAL::internal
#endif
