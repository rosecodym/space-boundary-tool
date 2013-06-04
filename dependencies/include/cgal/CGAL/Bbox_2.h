// Copyright (c) 1999,2004  
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
// $URL: svn+ssh://scm.gforge.inria.fr/svn/cgal/branches/releases/CGAL-4.1-branch/Kernel_23/include/CGAL/Bbox_2.h $
// $Id: Bbox_2.h 70567 2012-07-17 08:56:13Z gdamiand $
//
// Author(s)     : Andreas Fabri

#ifndef CGAL_BBOX_2_H
#define CGAL_BBOX_2_H

#include <CGAL/basic.h>
#include <CGAL/kernel_assertions.h>
#include <CGAL/IO/io.h>
#include <CGAL/Dimension.h>
#include <CGAL/array.h>

namespace CGAL {

template < typename T >
struct Simple_cartesian;

class Bbox_2
{
  typedef cpp0x::array<double, 4>            BBox_rep_2;

  BBox_rep_2 rep;

public:

  typedef Dimension_tag<2>  Ambient_dimension;
  typedef Dimension_tag<2>  Feature_dimension;

  typedef Simple_cartesian<double>  R;

             Bbox_2() {}

             Bbox_2(double x_min, double y_min,
                    double x_max, double y_max)
		 : rep(CGAL::make_array(x_min, y_min, x_max, y_max)) {}

  inline bool       operator==(const Bbox_2 &b) const;
  inline bool       operator!=(const Bbox_2 &b) const;

  inline int        dimension() const;
  inline double     xmin() const;
  inline double     ymin() const;
  inline double     xmax() const;
  inline double     ymax() const;

  inline double     max BOOST_PREVENT_MACRO_SUBSTITUTION (int i) const;
  inline double     min BOOST_PREVENT_MACRO_SUBSTITUTION (int i) const;

  inline Bbox_2     operator+(const Bbox_2 &b) const;

};

inline
double
Bbox_2::xmin() const
{ return rep[0]; }

inline
double
Bbox_2::ymin() const
{ return rep[1]; }

inline
double
Bbox_2::xmax() const
{ return rep[2]; }

inline
double
Bbox_2::ymax() const
{ return rep[3]; }

inline
bool
Bbox_2::operator==(const Bbox_2 &b) const
{
  return    xmin() == b.xmin() && xmax() == b.xmax()
         && ymin() == b.ymin() && ymax() == b.ymax();
}

inline
bool
Bbox_2::operator!=(const Bbox_2 &b) const
{
  return ! (b == *this);
}

inline
int
Bbox_2::dimension() const
{ return 2; }

inline
double
Bbox_2::min BOOST_PREVENT_MACRO_SUBSTITUTION (int i) const
{
  CGAL_kernel_precondition( (i == 0 ) || ( i == 1 ) );
  if(i == 0) { return xmin(); }
  return ymin();
}

inline
double
Bbox_2::max BOOST_PREVENT_MACRO_SUBSTITUTION (int i) const
{
  CGAL_kernel_precondition( (i == 0 ) || ( i == 1 ) );
  if(i == 0) { return xmax(); }
  return ymax();
}

inline
Bbox_2
Bbox_2::operator+(const Bbox_2 &b) const
{
  return Bbox_2((std::min)(xmin(), b.xmin()),
                (std::min)(ymin(), b.ymin()),
                (std::max)(xmax(), b.xmax()),
                (std::max)(ymax(), b.ymax()));
}

inline
bool
do_overlap(const Bbox_2 &bb1, const Bbox_2 &bb2)
{
    // check for emptiness ??
    if (bb1.xmax() < bb2.xmin() || bb2.xmax() < bb1.xmin())
        return false;
    if (bb1.ymax() < bb2.ymin() || bb2.ymax() < bb1.ymin())
        return false;
    return true;
}

inline
std::ostream&
operator<<(std::ostream &os, const Bbox_2 &b)
{
    switch(os.iword(IO::mode)) {
    case IO::ASCII :
        os << b.xmin() << ' ' << b.ymin() << ' '
           << b.xmax() << ' ' << b.ymax();
        break;
    case IO::BINARY :
        write(os, b.xmin());
        write(os, b.ymin());
        write(os, b.xmax());
        write(os, b.ymax());
        break;
    default:
        os << "Bbox_2(" << b.xmin() << ", " << b.ymin() << ", "
                        << b.xmax() << ", " << b.ymax() << ")";
        break;
    }
    return os;
}

inline
std::istream&
operator>>(std::istream &is, Bbox_2 &b)
{
    double xmin, ymin, xmax, ymax;

    switch(is.iword(IO::mode)) {
    case IO::ASCII :
        is >> xmin >> ymin >> xmax >> ymax;
        break;
    case IO::BINARY :
        read(is, xmin);
        read(is, ymin);
        read(is, xmax);
        read(is, ymax);
        break;
    }
    if (is)
      b = Bbox_2(xmin, ymin, xmax, ymax);
    return is;
}

} //namespace CGAL

#endif // CGAL_BBOX_2_H
