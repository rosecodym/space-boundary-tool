#pragma once

#include "precompiled.h"

class approximated_curve {
public:
	approximated_curve(
		const point_2 & p1, 
		const point_2 & p2,
		double area_on_left,
		double true_length)
		: p1_(p1.x(), p1.y(), 0.0),
		  p2_(p2.x(), p2.y(), 0.0),
		  n_(0.0, 0.0, 1.0),
		  length_ratio_(true_length),
		  area_(area_on_left)
	{ }

	direction_3	original_plane_normal() const { return n_; }
	double		true_area_on_left() const { return area_; }
	double		true_length_ratio() const { return length_ratio_; }

	bool matches(
		double x1, double y1, double z1,
		double x2, double y2, double z2,
		double eps) const;
	
	approximated_curve transformed(const transformation_3 & t) const {
		return approximated_curve(t(p1_), t(p2_), t(n_), length_ratio_, area_);
	}

private:
	approximated_curve(
		const point_3 & p1,
		const point_3 & p2,
		const direction_3 & normal,
		double r,
		double a)
		: p1_(p1), p2_(p2), n_(normal), length_ratio_(r), area_(a)
	{ }

	point_3 p1_;
	point_3 p2_;
	direction_3 n_;
	double length_ratio_;
	double area_;
};