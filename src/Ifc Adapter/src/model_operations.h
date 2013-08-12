#pragma once

#include "precompiled.h"

#include "approximated_curve.h"
#include "number_collection.h"
#include "sbt-ifcadapter.h"

namespace ifc_interface {

class model;

} // namespace ifc_interface

class unit_scaler;

ifcadapter_return_t extract_from_model(
	ifc_interface::model * m, 
	size_t * element_count, 
	element_info *** elements, 
	double ** composite_layer_dxs,
	double ** composite_layer_dys,
	double ** composite_layer_dzs,
	size_t * space_count,
	space_info *** spaces,
	const unit_scaler & scaler,
	void (*notify)(char *),
	void (*warn)(char *),
	const std::function<bool(const char *)> & element_filter,
	const std::function<bool(const char *)> & space_filter,
	number_collection<K> * c,
	std::vector<element_info *> * shadings,
	std::vector<approximated_curve> * approximated_curves);

ifcadapter_return_t add_to_model(
	ifc_interface::model * model,
	size_t sb_count,
	space_boundary ** space_boundaries,
	const unit_scaler & scaler,
	void (*msg_func)(char *),
	number_collection<iK> * c);