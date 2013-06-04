// Copyright (c) 2006-2008 Max-Planck-Institute Saarbruecken (Germany).
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
// $URL: svn+ssh://scm.gforge.inria.fr/svn/cgal/branches/releases/CGAL-4.1-branch/Number_types/include/CGAL/Sqrt_extension/convert_to_bfi.h $
// $Id: convert_to_bfi.h 67093 2012-01-13 11:22:39Z lrineau $
//
//
// Author(s)     : Michael Hemmer   <hemmer@mpi-inf.mpg.de>

// This files adds an optional static cache to convert_to_bfi for Sqrt_extension

#ifndef CGAL_SQRT_EXTENSION_CONVERT_TO_BFI_H
#define CGAL_SQRT_EXTENSION_CONVERT_TO_BFI_H

#include <CGAL/basic.h>
#include <CGAL/convert_to_bfi.h>
#include <CGAL/Coercion_traits.h>
#include <CGAL/Sqrt_extension/Sqrt_extension_type.h>


// Disbale SQRT_EXTENSION_TO_BFI_CACHE by default
#ifndef CGAL_USE_SQRT_EXTENSION_TO_BFI_CACHE
#define CGAL_USE_SQRT_EXTENSION_TO_BFI_CACHE 0
#endif

#if CGAL_USE_SQRT_EXTENSION_TO_BFI_CACHE

namespace CGAL {

namespace INTERN_SQRT_EXTENSION {
template <typename BFI, typename ROOT>
class Sqrt_extension_bfi_cache {
  typedef std::pair<long , ROOT> Input;
  typedef BFI                    Output;
  typedef typename Coercion_traits<ROOT,BFI>::Cast Cast;
  typedef typename Algebraic_structure_traits<BFI>::Sqrt Sqrt; 

  struct Creator : public std::unary_function<BFI,Input> {
    BFI operator()(const Input& pair){
      return Sqrt()(Cast()(pair.second)); 
    }
  };    

public:
  typedef Cache<Input,Output,Creator> Cache_type;
  static Cache_type cache; 
};
template <typename BFI, typename ROOT>
typename Sqrt_extension_bfi_cache<BFI,ROOT>::Cache_type 
Sqrt_extension_bfi_cache<BFI,ROOT>::cache;
} // namespace INTERN_SQRT_EXTENSION 


template <typename NT, typename ROOT, typename ACDE_TAG, typename FP_TAG>
typename Get_arithmetic_kernel<NT>::Arithmetic_kernel::Bigfloat_interval
convert_to_bfi(const CGAL::Sqrt_extension<NT,ROOT,ACDE_TAG,FP_TAG>& x) {
  typedef typename Get_arithmetic_kernel<NT>::Arithmetic_kernel AK;
  typedef typename AK::Bigfloat_interval BFI;
  typedef Bigfloat_interval_traits<BFI> BFIT;
  long precision = typename BFIT::Get_precision()();

  BFI result;
  if(x.is_extended()){
    typedef INTERN_SQRT_EXTENSION::Sqrt_extension_bfi_cache<BFI,ROOT> Get_cache;
    BFI a0(convert_to_bfi(x.a0()));
    BFI a1(convert_to_bfi(x.a1()));
    BFI root(Get_cache::cache(std::make_pair(precision,x.root())));  
    result = a0+a1*root;
  }else{
    result =  convert_to_bfi(x.a0());
  }
#ifndef NDEBUG
  BFI result_;
  typedef typename Get_arithmetic_kernel<NT>::Arithmetic_kernel AT;
  typedef typename AT::Bigfloat_interval BFI;
  if(x.is_extended()){
    BFI a0(convert_to_bfi(x.a0()));
    BFI a1(convert_to_bfi(x.a1()));
    BFI root(CGAL::sqrt(convert_to_bfi(x.root())));
    result_ = a0+a1*root;
  }else{
    result_ = convert_to_bfi(x.a0());
  }
  assert(lower(result) == lower(result_));
  assert(upper(result) == upper(result_));
#endif
  return result; 
}

} // namespace CGAL 

#endif 


#endif  // CGAL_SQRT_EXTENSION_CONVERT_TO_BFI_H
