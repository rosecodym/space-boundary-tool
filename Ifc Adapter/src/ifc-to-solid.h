#pragma once

#include "precompiled.h"

#include "cgal-typedefs.h"
#include "exact_surrogates.h"
#include "number_collection.h"
#include "sbt-ifcadapter.h"

class unit_scaler;

int ifc_to_solid(exact_solid * s, const cppw::Instance & inst, const unit_scaler & scaler, number_collection<K> * c); // nonzero return indicates failure