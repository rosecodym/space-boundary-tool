// Copyright (c) 2007-2011  INRIA Sophia-Antipolis (France).
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
// $URL: svn+ssh://scm.gforge.inria.fr/svn/cgal/branches/releases/CGAL-4.1-branch/Spatial_sorting/include/CGAL/hilbert_sort.h $
// $Id: hilbert_sort.h 67093 2012-01-13 11:22:39Z lrineau $
//
// Author(s)     : Christophe Delage
//               : Olivier Devillers

#ifndef CGAL_HILBERT_SORT_H
#define CGAL_HILBERT_SORT_H

#include <CGAL/basic.h>

#include <CGAL/Hilbert_policy_tags.h>
#include <CGAL/Hilbert_sort_2.h>
#include <CGAL/Hilbert_sort_3.h>
#include <CGAL/Hilbert_sort_d.h>

#include <boost/random.hpp>
#include <boost/random/linear_congruential.hpp>

#include <algorithm>

namespace CGAL {


namespace internal {

  template <class RandomAccessIterator, class Policy, class Kernel>
    void hilbert_sort (RandomAccessIterator begin, 
		       RandomAccessIterator end,
		       Policy /*policy*/,
		       const Kernel &k, typename Kernel::Point_2 *)
    {
        boost::rand48 random;
        boost::random_number_generator<boost::rand48> rng(random);
        std::random_shuffle(begin,end, rng);
	(Hilbert_sort_2<Kernel, Policy> (k))(begin, end);
    }
    
    template <class RandomAccessIterator, class Policy, class Kernel>
      void hilbert_sort (RandomAccessIterator begin, 
			 RandomAccessIterator end,
			 Policy /*policy*/,
			 const Kernel &k, typename Kernel::Point_3 *)
    {
        boost::rand48 random;
        boost::random_number_generator<boost::rand48> rng(random);
        std::random_shuffle(begin,end, rng);
        (Hilbert_sort_3<Kernel, Policy> (k))(begin, end);
    }

    template <class RandomAccessIterator, class Policy, class Kernel>
      void hilbert_sort (RandomAccessIterator begin,
			 RandomAccessIterator end,
			 Policy /*policy*/,
			 const Kernel &k, typename Kernel::Point_d *)
    {
        boost::rand48 random;
        boost::random_number_generator<boost::rand48> rng(random);
        std::random_shuffle(begin,end, rng);
        (Hilbert_sort_d<Kernel, Policy> (k))(begin, end);
    }
    


}

template <class RandomAccessIterator>
void hilbert_sort (RandomAccessIterator begin, RandomAccessIterator end)
{
  hilbert_sort (begin, end, Hilbert_sort_median_policy());
}

template <class RandomAccessIterator, class Policy>
void hilbert_sort (RandomAccessIterator begin, RandomAccessIterator end,
		   Policy policy)
{
    typedef std::iterator_traits<RandomAccessIterator> ITraits;
    typedef typename ITraits::value_type               value_type;
    typedef CGAL::Kernel_traits<value_type>            KTraits;
    typedef typename KTraits::Kernel                   Kernel;

    internal::hilbert_sort(begin, end, policy, Kernel(), 
				  static_cast<value_type *> (0));
}


template <class RandomAccessIterator, class Policy, class Kernel>
void hilbert_sort (RandomAccessIterator begin, RandomAccessIterator end,
		   Policy policy, const Kernel &k )
{
    typedef std::iterator_traits<RandomAccessIterator> ITraits;
    typedef typename ITraits::value_type               value_type;

    internal::hilbert_sort(begin, end, 
			   policy, k, static_cast<value_type *> (0));
}

} // namespace CGAL

#endif//CGAL_HILBERT_SORT_H

