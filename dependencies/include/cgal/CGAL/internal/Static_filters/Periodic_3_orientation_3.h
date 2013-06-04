// Copyright (c) 2001,2004,2008-2009   INRIA Sophia-Antipolis (France).
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
// $URL: svn+ssh://scm.gforge.inria.fr/svn/cgal/branches/releases/CGAL-4.1-branch/Periodic_3_triangulation_3/include/CGAL/internal/Static_filters/Periodic_3_orientation_3.h $
// $Id: Periodic_3_orientation_3.h 67117 2012-01-13 18:14:48Z lrineau $
// 
//
// Author(s)     : Sylvain Pion <Sylvain.Pion@sophia.inria.fr>
//                 Manuel Caroli <Manuel.Caroli@sophia.inria.fr>

#ifndef CGAL_INTERNAL_STATIC_FILTERS_PERIODIC_3_ORIENTATION_3_H
#define CGAL_INTERNAL_STATIC_FILTERS_PERIODIC_3_ORIENTATION_3_H

#include <CGAL/Profile_counter.h>
#include <CGAL/internal/Static_filters/Static_filter_error.h>

#include <CGAL/Periodic_3_offset_3.h>

#include <cmath>

namespace CGAL { namespace internal { namespace Static_filters_predicates {

template < typename K_base >
class Periodic_3_orientation_3
  : public K_base::Orientation_3
{
  typedef typename K_base::Orientation_3    Base;
  
  typedef typename K_base::Point_3          Point_3;
  typedef typename K_base::Vector_3         Vector_3;
  typedef typename K_base::Iso_cuboid_3     Iso_cuboid_3;
  typedef typename K_base::Sphere_3         Sphere_3;
  typedef CGAL::Periodic_3_offset_3         Offset;

public:
  const Iso_cuboid_3 * const _dom;

public:
 typedef typename Base::result_type  result_type;

 template <class EX, class AP>
 Periodic_3_orientation_3(const Iso_cuboid_3 * const dom,
     const EX * dom_e, const AP * dom_f) : Base(dom_e,dom_f), _dom(dom) {
 }

#ifndef CGAL_CFG_MATCHING_BUG_6
  using Base::operator();
#else 
  result_type
  operator()(const Vector_3& u, const Vector_3& v, const Vector_3& w) const
  { 
    return Base::operator()(u,v,w);
  }  

  result_type
  operator()(const Sphere_3& s) const
  { 
    return Base::operator()(s);
  }
#endif

  result_type 
  operator()(const Point_3 &p, const Point_3 &q,
	     const Point_3 &r, const Point_3 &s) const
  {
      CGAL_PROFILER("Periodic_3_orientation_3 calls");

      double px, py, pz, qx, qy, qz, rx, ry, rz, sx, sy, sz;

      if (fit_in_double(p.x(), px) && fit_in_double(p.y(), py) &&
          fit_in_double(p.z(), pz) &&
          fit_in_double(q.x(), qx) && fit_in_double(q.y(), qy) &&
          fit_in_double(q.z(), qz) &&
          fit_in_double(r.x(), rx) && fit_in_double(r.y(), ry) &&
          fit_in_double(r.z(), rz) &&
          fit_in_double(s.x(), sx) && fit_in_double(s.y(), sy) &&
          fit_in_double(s.z(), sz))
      {
          CGAL_PROFILER("Periodic_3_orientation_3 semi-static attempts");

          double pqx = qx - px;
          double pqy = qy - py;
          double pqz = qz - pz;
          double prx = rx - px;
          double pry = ry - py;
          double prz = rz - pz;
          double psx = sx - px;
          double psy = sy - py;
          double psz = sz - pz;

          // Then semi-static filter.
          double maxx = CGAL::abs(pqx);
          double maxy = CGAL::abs(pqy);
          double maxz = CGAL::abs(pqz);

          double aprx = CGAL::abs(prx);
          double apsx = CGAL::abs(psx);

          double apry = CGAL::abs(pry);
          double apsy = CGAL::abs(psy);

          double aprz = CGAL::abs(prz);
          double apsz = CGAL::abs(psz);

          if (maxx < aprx) maxx = aprx;
          if (maxx < apsx) maxx = apsx;
          if (maxy < apry) maxy = apry;
          if (maxy < apsy) maxy = apsy;
          if (maxz < aprz) maxz = aprz;
          if (maxz < apsz) maxz = apsz;
          double eps = 5.1107127829973299e-15 * maxx * maxy * maxz;
          double det = CGAL::determinant(pqx, pqy, pqz,
                                         prx, pry, prz,
                                         psx, psy, psz);

          // Sort maxx < maxy < maxz.
          if (maxx > maxz)
              std::swap(maxx, maxz);
          if (maxy > maxz)
              std::swap(maxy, maxz);
          else if (maxy < maxx)
              std::swap(maxx, maxy);

          // Protect against underflow in the computation of eps.
          if (maxx < 1e-97) /* cbrt(min_double/eps) */ {
            if (maxx == 0)
              return ZERO;
          }
          // Protect against overflow in the computation of det.
          else if (maxz < 1e102) /* cbrt(max_double [hadamard]/4) */ {
            if (det > eps)  return POSITIVE;
            if (det < -eps) return NEGATIVE;
          }

          CGAL_PROFILER("Periodic_3_orientation_3 semi-static failures");
      }

      return Base::operator()(p, q, r, s);
  }

  result_type
  operator()(const Point_3 &p, const Point_3 &q,
      const Point_3 &r, const Point_3 &s,
      const Offset &o_p, const Offset &o_q,
      const Offset &o_r, const Offset &o_s) const {

      CGAL_PROFILER("Periodic_3_orientation_3 calls");

      double px, py, pz, qx, qy, qz, rx, ry, rz, sx, sy, sz;
      double domxmax, domxmin, domymax, domymin, domzmax, domzmin;
      int opx = o_p.x();
      int opy = o_p.y();
      int opz = o_p.z();

      if (fit_in_double(p.x(), px) && fit_in_double(p.y(), py) &&
          fit_in_double(p.z(), pz) &&
          fit_in_double(q.x(), qx) && fit_in_double(q.y(), qy) &&
          fit_in_double(q.z(), qz) &&
          fit_in_double(r.x(), rx) && fit_in_double(r.y(), ry) &&
          fit_in_double(r.z(), rz) &&
          fit_in_double(s.x(), sx) && fit_in_double(s.y(), sy) &&
          fit_in_double(s.z(), sz) &&
	  fit_in_double(_dom->xmax(), domxmax) &&
	  fit_in_double(_dom->xmin(), domxmin) &&
	  fit_in_double(_dom->ymax(), domymax) &&
	  fit_in_double(_dom->ymin(), domymin) &&
	  fit_in_double(_dom->zmax(), domzmax) &&
	  fit_in_double(_dom->zmin(), domzmin))
      {
          CGAL_PROFILER("Periodic_3_orientation_3 semi-static attempts");

	  double domx = domxmax - domxmin;
	  double domy = domymax - domymin;
	  double domz = domzmax - domzmin;

          double pqx = qx - px + domx * ( o_q.x() - opx );
          double pqy = qy - py + domy * ( o_q.y() - opy );
          double pqz = qz - pz + domz * ( o_q.z() - opz );
          double prx = rx - px + domx * ( o_r.x() - opx );
          double pry = ry - py + domy * ( o_r.y() - opy );
          double prz = rz - pz + domz * ( o_r.z() - opz );
          double psx = sx - px + domx * ( o_s.x() - opx );
          double psy = sy - py + domy * ( o_s.y() - opy );
          double psz = sz - pz + domz * ( o_s.z() - opz );

          // Then semi-static filter.
          double maxx = CGAL::abs(pqx);
          double maxy = CGAL::abs(pqy);
          double maxz = CGAL::abs(pqz);

          double aprx = CGAL::abs(prx);
          double apry = CGAL::abs(pry);
          double aprz = CGAL::abs(prz);

          double apsx = CGAL::abs(psx);
          double apsy = CGAL::abs(psy);
          double apsz = CGAL::abs(psz);

          if (maxx < aprx) maxx = aprx;
          if (maxx < apsx) maxx = apsx;

          if (maxy < apry) maxy = apry;
          if (maxy < apsy) maxy = apsy;

          if (maxz < aprz) maxz = aprz;
          if (maxz < apsz) maxz = apsz;
          double eps = 4.111024169857068197e-15 * maxx * maxy * maxz;
          double det = CGAL::determinant(pqx, pqy, pqz,
                                         prx, pry, prz,
                                         psx, psy, psz);

          // Sort maxx < maxy < maxz.
          if (maxx > maxz)
              std::swap(maxx, maxz);
          if (maxy > maxz)
              std::swap(maxy, maxz);
          else if (maxy < maxx)
              std::swap(maxx, maxy);

          // Protect against underflow in the computation of eps.
          if (maxx < 1e-97) /* cbrt(min_double/eps) */ {
            if (maxx == 0)
              return ZERO;
          }
          // Protect against overflow in the computation of det.
          else if (maxz < 1e102) /* cbrt(max_double [hadamard]/4) */ {
            if (det > eps)  return POSITIVE;
            if (det < -eps) return NEGATIVE;
          }

          CGAL_PROFILER("Periodic_3_orientation_3 semi-static failures");
      }

      return Base::operator()(p,q,r,s,o_p,o_q,o_r,o_s);
  }

  // Computes the epsilon for Periodic_3_orientation_3.
  static double compute_epsilon()
  {
    typedef Static_filter_error F;
    F t1 = F(1, F::ulp()/4);         // First translation
    F det = CGAL::determinant(t1, t1, t1,
                              t1, t1, t1,
                              t1, t1, t1); // Full det
    double err = det.error();
    err += err * 2 * F::ulp(); // Correction due to "eps * maxx * maxy...".
    std::cerr << "*** epsilon for Periodic_3_orientation_3 = " << err 
	      << std::endl;
    return err;
  }

};

} } } // namespace CGAL::internal::Static_filters_predicates

#endif // CGAL_INTERNAL_STATIC_FILTERS_PERIODIC_3_ORIENTATION_3_H
