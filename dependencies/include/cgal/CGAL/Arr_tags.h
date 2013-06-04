// Copyright (c) 2007,2009,2010,2011 Tel-Aviv University (Israel).
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
// $URL: svn+ssh://scm.gforge.inria.fr/svn/cgal/branches/releases/CGAL-4.1-branch/Arrangement_on_surface_2/include/CGAL/Arr_tags.h $
// $Id: Arr_tags.h 67117 2012-01-13 18:14:48Z lrineau $
// 
//
// Author(s): Efi Fogel         <efif@post.tau.ac.il>
//            Eric Berberich    <ericb@post.tau.ac.il>

#ifndef CGAL_ARR_TAGS_H
#define CGAL_ARR_TAGS_H

#include <boost/type_traits.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/or.hpp>
#include <boost/mpl/not.hpp>
#include <boost/mpl/logical.hpp>
#include <boost/mpl/has_xxx.hpp>

/*! \file
 * Definition of the tags for the arrangement package.
 */

namespace CGAL {

struct Arr_boundary_side_tag {};
struct Arr_oblivious_side_tag : public virtual Arr_boundary_side_tag {};
struct Arr_open_side_tag : public virtual Arr_oblivious_side_tag {};
struct Arr_closed_side_tag : public virtual Arr_oblivious_side_tag {};
struct Arr_contracted_side_tag : public virtual Arr_oblivious_side_tag {};
struct Arr_identified_side_tag : public virtual Arr_oblivious_side_tag {};

BOOST_MPL_HAS_XXX_TRAIT_DEF(Left_side_category)
BOOST_MPL_HAS_XXX_TRAIT_DEF(Bottom_side_category)
BOOST_MPL_HAS_XXX_TRAIT_DEF(Top_side_category)
BOOST_MPL_HAS_XXX_TRAIT_DEF(Right_side_category)

namespace internal {

//! type to provide left side tag (is oblivious if not existing)
template < class Traits_, bool B >
struct Get_left_side_category { 
};

template < class Traits_ >
struct Get_left_side_category< Traits_, true > {
  typedef typename Traits_::Left_side_category Category;
};

template < class Traits_ >
struct Get_left_side_category< Traits_, false > {
  typedef Arr_oblivious_side_tag Category;
};

template < class Traits_ >
struct Arr_complete_left_side_category {

public:

  typedef Traits_ Traits;

  typedef typename
  Get_left_side_category< Traits, has_Left_side_category< Traits >::value >::Category Category;
};

template < class GeometryTraits_2, bool b > 
struct Validate_left_side_category {};

template < class GeometryTraits_2 >
struct Validate_left_side_category< GeometryTraits_2, true > {
  void missing__Left_side_category() {}
};

template < class GeometryTraits_2 >
struct Validate_left_side_category< GeometryTraits_2, false > {
  void missing__Left_side_category()
  { 
    bool 
      missing__Left_side_category__assuming__Arr_oblivious_side_tag__instead; 
  }
};


//! type to provide bottom side tag (is oblivious if not existing)
template < class Traits_, bool B >
struct Get_bottom_side_category { 
};

template < class Traits_ >
struct Get_bottom_side_category< Traits_, true > {
  typedef typename Traits_::Bottom_side_category Category;
};

template < class Traits_ >
struct Get_bottom_side_category< Traits_, false > {
  typedef Arr_oblivious_side_tag Category;
};

template < class Traits_ >
struct Arr_complete_bottom_side_category {

public:

  typedef Traits_ Traits;

  typedef typename
  Get_bottom_side_category< Traits, has_Bottom_side_category< Traits >::value >::Category 
  Category;
};

template < class GeometryTraits_2, bool b > 
struct Validate_bottom_side_category {};

template < class GeometryTraits_2 >
struct Validate_bottom_side_category< GeometryTraits_2, true > {
  void missing__Bottom_side_category() {}
};

template < class GeometryTraits_2 >
struct Validate_bottom_side_category< GeometryTraits_2, false > {
  void missing__Bottom_side_category()
  { 
    bool 
      missing__Bottom_side_category__assuming__Arr_oblivious_side_tag__instead; 
  }
};


//! type to provide top side tag (is oblivious if not existing)
template < class Traits_, bool B >
struct Get_top_side_category { 
};

template < class Traits_ >
struct Get_top_side_category< Traits_, true > {
  typedef typename Traits_::Top_side_category Category;
};

template < class Traits_ >
struct Get_top_side_category< Traits_, false > {
  typedef Arr_oblivious_side_tag Category;
};

template < class Traits_ >
struct Arr_complete_top_side_category {

public:

  typedef Traits_ Traits;

  typedef typename
  Get_top_side_category< Traits, has_Top_side_category< Traits >::value >::Category Category;
};

template < class GeometryTraits_2, bool b > 
struct Validate_top_side_category {};

template < class GeometryTraits_2 >
struct Validate_top_side_category< GeometryTraits_2, true > {
  void missing__Top_side_category() {}
};

template < class GeometryTraits_2 >
struct Validate_top_side_category< GeometryTraits_2, false > {
  void missing__Top_side_category()
  { 
    bool missing__Top_side_category__assuming__Arr_oblivious_side_tag__instead; 
  }
};


//! type to provide right side tag (is oblivious if not existing)
template < class Traits_, bool B >
struct Get_right_side_category { 
};

template < class Traits_ >
struct Get_right_side_category< Traits_, true > {
  typedef typename Traits_::Right_side_category Category;
};

template < class Traits_ >
struct Get_right_side_category< Traits_, false > {
  typedef Arr_oblivious_side_tag Category;
};

template < class Traits_ >
struct Arr_complete_right_side_category {

public:

  typedef Traits_ Traits;

  typedef typename
  Get_right_side_category< Traits, has_Right_side_category< Traits >::value >::Category 
  Category;
};

template < class GeometryTraits_2, bool b > 
struct Validate_right_side_category {};

template < class GeometryTraits_2 >
struct Validate_right_side_category< GeometryTraits_2, true > {
  void missing__Right_side_category() {}
};

template < class GeometryTraits_2 >
struct Validate_right_side_category< GeometryTraits_2, false > {
  void missing__Right_side_category()
  { 
    bool 
      missing__Right_side_category__assuming__Arr_oblivious_side_tag__instead; 
  }
};




} // namespace internal
  
struct Arr_all_sides_oblivious_tag {};
struct Arr_not_all_sides_oblivious_tag {};

struct Arr_all_sides_open_tag {};

struct Arr_all_sides_non_open_tag {};
struct Arr_not_all_sides_non_open_tag {};


/*!\brief Struct to determine whether all side tags are "oblivious"
 */
template < class ArrLeftSideCategory, class ArrBottomSideCategory, 
           class ArrTopSideCategory, class ArrRightSideCategory >
struct Arr_are_all_sides_oblivious_tag {

public:

  //! This instance's first template parameter
  typedef ArrLeftSideCategory   Left_side_category;
  
  //! This instance's second template parameter
  typedef ArrBottomSideCategory Bottom_side_category;
  
  //! This instance's third template parameter
  typedef ArrTopSideCategory    Top_side_category;
  
  //! This instance's fourth template parameter
  typedef ArrRightSideCategory  Right_side_category;
  
private:
  
  typedef boost::mpl::bool_< true > true_;
  typedef boost::mpl::bool_< false > false_;
  
  typedef boost::mpl::if_< 
       boost::is_same< Left_side_category, Arr_oblivious_side_tag >,
       true_, false_ > 
  Left_oblivious;

  typedef boost::mpl::if_< 
       boost::is_same< Bottom_side_category, Arr_oblivious_side_tag >,
       true_, false_ > 
  Bottom_oblivious;

  typedef boost::mpl::if_< 
       boost::is_same< Top_side_category, Arr_oblivious_side_tag >,
       true_, false_ > 
  Top_oblivious;

  typedef boost::mpl::if_< 
       boost::is_same< Right_side_category, Arr_oblivious_side_tag >,
       true_, false_ > 
  Right_oblivious;
  
public:
  
  /*!\brief
   * boolean tag that is Arr_all_sides_oblivious_tag if all sides are 
   * oblivious, otherwise Arr_not_all_sides_oblivious_tag
   */
  typedef typename boost::mpl::if_< 
                           boost::mpl::and_< Left_oblivious, Bottom_oblivious, 
                                             Top_oblivious, Right_oblivious >,
                           Arr_all_sides_oblivious_tag,
                           Arr_not_all_sides_oblivious_tag >::type result;

};

/*!\brief Struct to determine whether all side tags are "non-open"
 */
template < class ArrLeftSideCategory, class ArrBottomSideCategory, 
           class ArrTopSideCategory, class ArrRightSideCategory >
struct Arr_are_all_sides_non_open_tag {

public:

  //! This instance's first template parameter
  typedef ArrLeftSideCategory   Left_side_category;
  
  //! This instance's second template parameter
  typedef ArrBottomSideCategory Bottom_side_category;
  
  //! This instance's third template parameter
  typedef ArrTopSideCategory    Top_side_category;
  
  //! This instance's fourth template parameter
  typedef ArrRightSideCategory  Right_side_category;
  
private:
  
  typedef boost::mpl::bool_< true > true_;
  typedef boost::mpl::bool_< false > false_;
  
  typedef boost::mpl::if_< 
       boost::is_same< Left_side_category, Arr_open_side_tag >,
       true_, false_ > 
  Left_open;

  typedef boost::mpl::if_< 
       boost::is_same< Bottom_side_category, Arr_open_side_tag >,
       true_, false_ > 
  Bottom_open;

  typedef boost::mpl::if_< 
       boost::is_same< Top_side_category, Arr_open_side_tag >,
       true_, false_ > 
  Top_open;

  typedef boost::mpl::if_< 
       boost::is_same< Right_side_category, Arr_open_side_tag >,
       true_, false_ > 
  Right_open;
  
public:
  
  /*!\brief
   * boolean tag that is Arr_all_sides_non_open_tag if all sides are non-open, 
   * otherwise Arr_not_all_sides_non_open_tag
   */
  typedef typename boost::mpl::if_<
      boost::mpl::and_< boost::mpl::not_< Left_open >, 
                        boost::mpl::not_< Bottom_open >, 
                        boost::mpl::not_< Top_open >, 
                        boost::mpl::not_< Right_open > >,
      Arr_all_sides_non_open_tag,
      Arr_not_all_sides_non_open_tag >::type result;
};


/*!\brief Struct to check consistent tagging of identifications
 */
template < class ArrLeftSideCategory, class ArrBottomSideCategory, 
           class ArrTopSideCategory, class ArrRightSideCategory >
struct Arr_sane_identified_tagging {

public:
  
  //! This instance's first template parameter
  typedef ArrLeftSideCategory   Left_side_category;
  
  //! This instance's second template parameter
  typedef ArrBottomSideCategory Bottom_side_category;
  
  //! This instance's third template parameter
  typedef ArrTopSideCategory    Top_side_category;
  
  //! This instance's fourth template parameter
  typedef ArrRightSideCategory  Right_side_category;
  
private:
  
  typedef boost::mpl::bool_< true > true_;
  typedef boost::mpl::bool_< false > false_;
  
  typedef boost::mpl::if_< 
       boost::is_same< Left_side_category, Arr_identified_side_tag >,
       true_, false_ > 
  Left_identified;

  typedef boost::mpl::if_<
       boost::is_same< Bottom_side_category, Arr_identified_side_tag >,
       true_, false_ > 
  Bottom_identified;

  typedef boost::mpl::if_< 
       boost::is_same< Top_side_category, Arr_identified_side_tag >,
       true_, false_ > 
  Top_identified;
  
  typedef boost::mpl::if_< 
       boost::is_same< Right_side_category, Arr_identified_side_tag >,
       true_, false_ > 
  Right_identified;

  typedef boost::mpl::and_< Left_identified, Right_identified > LR_identified;

  typedef boost::mpl::and_< Bottom_identified, Top_identified > BT_identified;

  typedef boost::mpl::and_< boost::mpl::not_< Left_identified>,
                            boost::mpl::not_< Right_identified > >
  LR_non_identified;

  typedef boost::mpl::and_< boost::mpl::not_< Bottom_identified >, 
                            boost::mpl::not_< Top_identified > > 
  BT_non_identified;
  
  typedef boost::mpl::or_< LR_identified, LR_non_identified > LR_ok;
  typedef boost::mpl::or_< BT_identified, BT_non_identified > BT_ok;
  
public:
  
  /*!\brief
   * boolean tag that is bool_<true> if opposite sides are either 
   * both identified or both non-identified,
   * otherwise bool_<false>
   */
  typedef boost::mpl::and_< LR_ok, BT_ok > result;

};

} //namespace CGAL

#endif
