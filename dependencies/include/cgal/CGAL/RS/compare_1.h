// Copyright (c) 2006-2008 Inria Lorraine (France). All rights reserved.
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
// $URL: svn+ssh://scm.gforge.inria.fr/svn/cgal/branches/releases/CGAL-4.1-branch/Algebraic_kernel_d/include/CGAL/RS/compare_1.h $
// $Id: compare_1.h 68103 2012-03-20 13:09:04Z eric $
//
// Author: Luis Peñaranda <luis.penaranda@gmx.com>

#ifndef CGAL_RS_COMPARE_1_H
#define CGAL_RS_COMPARE_1_H

#include <mpfr.h>
#include <CGAL/RS/polynomial_1.h>
#include <CGAL/RS/algebraic_1.h>
#include <CGAL/RS/polynomial_1_utils.h>
#include <CGAL/RS/sign_1.h>
#include <CGAL/RS/refine_1_rs.h>

// default refinement and sign functions
#define CGALRS_REFINE_N(A,N)        RS3::refine_1(A,N)
#define CGALRS_REFSTEPS             4
#define CGALRS_SIGNAT(P,M)          RSSign::signat(P,M)

namespace CGAL{
namespace RS_COMPARE{

// compare two algebraic numbers, knowing they are not equal
//template <class _Gcd_policy>
inline 
Comparison_result
compare_1_unequal(const Algebraic_1 &r1,const Algebraic_1 &r2){
        /*typedef _Gcd_policy     Gcd;
        mp_prec_t prec1=r1.get_prec();
        mp_prec_t prec2=r2.get_prec();*/
        CGALRS_REFINE_N(r1,CGALRS_REFSTEPS);
        CGALRS_REFINE_N(r2,CGALRS_REFSTEPS);
        while(r1.overlaps(r2)){
                /*if(prec1<prec2 || r2.lefteval()==ZERO){
                        CGALRS_REFINE_N(r1,CGALRS_REFSTEPS);
                        prec1=r1.get_prec();
                }else{
                        CGALRS_REFINE_N(r2,CGALRS_REFSTEPS);
                        prec2=r2.get_prec();
                }
                */
                CGALRS_REFINE_N(r1,CGALRS_REFSTEPS);
                CGALRS_REFINE_N(r2,CGALRS_REFSTEPS);
        }
        return(mpfr_less_p(r1.right(),r2.left())?SMALLER:LARGER);
}

template <class _Gcd_policy>
Comparison_result
compare_1(const Algebraic_1 &r1,const Algebraic_1 &r2){
        typedef _Gcd_policy     Gcd;
        /*if(r1.pol()==r2.pol())
                return(r1.nr()!=r2.nr()?(r1.nr()<r2.nr()?SMALLER:LARGER):EQUAL);
        */
        if(mpfr_lessequal_p(r1.left(),r2.left())){
                if(mpfr_less_p(r1.right(),r2.left()))
                        return SMALLER;
        }else{
                if(mpfr_less_p(r2.right(),r1.left()))
                        return LARGER;
        }
        RS_polynomial_1 gcd=Gcd()(r1.pol(),r2.pol());
        if(!gcd.get_degree())
                return RS_COMPARE::compare_1_unequal/*<Gcd>*/(r1,r2);
        Sign sleft,sright;
        if(mpfr_greater_p(r1.left(),r2.left()))
                sleft=CGALRS_SIGNAT(gcd,r1.left());
        else
                sleft=CGALRS_SIGNAT(gcd,r2.left());
        if(sleft==ZERO)
                return EQUAL;
        if(mpfr_less_p(r1.right(),r2.right()))
                sright=CGALRS_SIGNAT(gcd,r1.right());
        else
                sright=CGALRS_SIGNAT(gcd,r2.right());
        if(sleft!=sright)
                return EQUAL;
        else
                return RS_COMPARE::compare_1_unequal/*<Gcd>*/(r1,r2);
}

} // namespace RS_COMPARE
} // namespace CGAL

#undef CGALRS_REFINE_N
#undef CGALRS_REFSTEPS
#undef CGALRS_SIGNAT

#endif  // CGAL_RS_COMPARE_1_H
