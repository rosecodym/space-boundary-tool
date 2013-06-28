#pragma once

#include "precompiled.h"

class unit_scaler {
private:
	double length_factor;

	unit_scaler() : length_factor(1.0) { }

public:
	unit_scaler(double length_units_per_meter);

	double length_in(double val) const { return val * length_factor; }
	double length_out(double val) const { return val / length_factor; }

	static const unit_scaler identity_scaler;
};