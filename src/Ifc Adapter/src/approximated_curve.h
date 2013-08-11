#pragma once

#include "precompiled.h"

class approximated_curve {
public:
	approximated_curve(
		const point_2 & p1, 
		const point_2 & p2,
		double area_on_left,
		double true_length);

	direction_3	original_plane_normal() const { return n_; }
	double		true_area_on_left() const { return area_; }
	double		true_length_ratio() const { return length_ratio_; }

	bool matches(
		double x1, double y1, double z1,
		double x2, double y2, double z2,
		double eps) const;

	approximated_curve reversed() const {
		return approximated_curve(p2_, p1_, -n_, length_ratio_, -area_);
	}
	
	approximated_curve transformed(const transformation_3 & t) const {
		return approximated_curve(t(p1_), t(p2_), t(n_), length_ratio_, area_);
	}

private:
	approximated_curve(
		const point_3 & p1,
		const point_3 & p2,
		const direction_3 & normal,
		double r,
		double a);

	point_3 p1_;
	point_3 p2_;
	direction_3 n_;
	double length_ratio_;
	double area_;

#ifndef NDEBUG
	double dp1_[3];
	double dp2_[3];
#endif
};