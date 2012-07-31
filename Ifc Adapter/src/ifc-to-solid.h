#pragma once

#include "precompiled.h"

#include "exact_surrogates.h"
#include "sbt-ifcadapter.h"

class unit_scaler;

int ifc_to_solid(exact_solid * s, const cppw::Instance & inst, const unit_scaler & scaler); // nonzero return indicates failure