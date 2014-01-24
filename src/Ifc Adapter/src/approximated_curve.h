#pragma once

#include "precompiled.h"

class approximated_curve {
public:
	enum match_type {
		NO_MATCH = 0,
		FORWARD_MATCH,
		REVERSE_MATCH
	};

	approximated_curve(
		const point_3 & p1, 
		const point_3 & p2,
		const direction_3 & normal,
		double area_on_left,
		double length_ratio);

	direction_3	original_plane_normal() const { return n_; }
	double		true_area_on_left() const { return area_; }
	double		true_length_ratio() const { return length_ratio_; }

	match_type matches(
		double x1, double y1, double z1,
		double x2, double y2, double z2,
		double eps) const;

	approximated_curve reversed() const {
		return approximated_curve(p2_, p1_, -n_, -area_, length_ratio_);
	}
	
	approximated_curve transformed(const transformation_3 & t) const {
		return approximated_curve(t(p1_), t(p2_), t(n_), area_, length_ratio_);
	}

private:
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