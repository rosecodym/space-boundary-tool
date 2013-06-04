#pragma once

#include "precompiled.h"

#include <cpp_edmi.h>

class unit_scaler {
private:
	double length_factor;

	unit_scaler() : length_factor(1.0) { }

public:
	unit_scaler(const cppw::Open_model & model);

	double length_in(double val) const { return val * length_factor; }
	double length_out(double val) const { return val / length_factor; }

	static const unit_scaler identity_scaler;
};