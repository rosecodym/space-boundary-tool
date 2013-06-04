#pragma once

#include "precompiled.h"

#include "area.h"

class space;

namespace traversal {

namespace impl {

class space_face {
private:
	bool sense_;
	NT h_;
	geometry_2d::area a_;
	NT orig_reg_area_;
	const space * space_;
public:
	space_face(const space * sp, const oriented_area & geometry) 
		: sense_(geometry.sense()), 
		  h_(geometry.height()), 
		  a_(geometry.area_2d()), 
		  orig_reg_area_(geometry.area_2d().regular_area()),
		  space_(sp) { }

	bool sense() const { return sense_; }
	const NT & height() const { return h_; }
	const geometry_2d::area & face_area() const { return a_; }
	const NT & starting_regular_area() const { return orig_reg_area_; }
	const space * bounded_space() const { return space_; }

	void remove_area(const geometry_2d::area & other) { a_ -= other; }
};

} // namespace impl

} // namespace stacking