#pragma once

#include "precompiled.h"

#include "exact_surrogates.h"
#include "cgal-typedefs.h"
#include "sbt-ifcadapter.h"

class unit_scaler;

exact_face ifc_to_face(const cppw::Instance & inst, const unit_scaler & s);
void transform_according_to(exact_face * f, const cppw::Select & sel, const unit_scaler & s);
bool normal_matches_extrusion(const exact_face & face, const direction_3 & dir);