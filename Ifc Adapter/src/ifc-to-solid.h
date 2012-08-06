#pragma once

#include "precompiled.h"

#include "exact_surrogates.h"
#include "sbt-ifcadapter.h"

class number_collection;
class unit_scaler;

int ifc_to_solid(exact_solid * s, const cppw::Instance & inst, const unit_scaler & scaler, number_collection * c); // nonzero return indicates failure