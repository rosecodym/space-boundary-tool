#pragma once

#include "precompiled.h"

#include "approximated_curve.h"
#include "number_collection.h"

namespace ifc_interface {

class ifc_object;

} // namespace ifc_interface

class unit_scaler;

namespace internal_geometry {

std::tuple<std::vector<point_3>, std::vector<approximated_curve>> 
build_polyloop(
	const ifc_interface::ifc_object & obj, 
	const unit_scaler & s, 
	number_collection<K> * c);

} // namespace internal_geometry