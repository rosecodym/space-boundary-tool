#pragma once

#include "precompiled.h"

class unit_scaler {
private:
	double lupm_;

	unit_scaler() : lupm_(1.0) { }

public:
	unit_scaler(double length_units_per_meter)
		: lupm_(length_units_per_meter) { }

	double length_in(double val) const { return val / lupm_; }
	double length_out(double val) const { return val * lupm_; }

	static const unit_scaler identity_scaler;
};