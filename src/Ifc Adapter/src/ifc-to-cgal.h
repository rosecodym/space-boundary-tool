#pragma once

#include "precompiled.h"

#include "number_collection.h"
#include "unit_scaler.h"

namespace ifc_interface {

class ifc_object;

} // namespace ifc_interface

template <typename KernelT>
CGAL::Point_3<KernelT> build_point(
	const ifc_interface::ifc_object & obj,
	const unit_scaler & s,
	number_collection<KernelT> * c)
{
	assert(is_kind_of(obj, "IfcCartesianPoint"));
	double x, y, z;
	triple_field(obj, "Coordinates", &x, &y, &z);
	return c->request_point(s.length_in(x), s.length_in(y), s.length_in(z));
}

template <typename KernelT>
CGAL::Direction_3<KernelT> build_direction(
	const ifc_interface::ifc_object & obj,
	number_collection<KernelT> * c)
{
	assert(is_kind_of(obj, "IfcDirection"));
	double dx, dy, dz;
	triple_field(obj, "DirectionRatios", &dx, &dy, &dz);
	return c->request_direction(dx, dy, dz);
}

template <typename KernelT>
CGAL::Aff_transformation_3<KernelT> build_transformation(
	const ifc_interface::ifc_object * obj,
	const unit_scaler & s,
	number_collection<KernelT> * c)
{
	typedef CGAL::Aff_transformation_3<KernelT> result_t;
	if (obj) {
		if (is_instance_of(*obj, "IfcLocalPlacement")) {
			ifc_interface::ifc_object * rel_to;
			object_field(*obj, "PlacementRelTo", &rel_to);
			ifc_interface::ifc_object * from;
			object_field(*obj, "RelativePlacement", &from);
			return 
				build_transformation(rel_to, s, c) *
				build_transformation(from, s, c);
		}
		else if (is_instance_of(*obj, "IfcAxis2Placement2D")) {
			ifc_interface::ifc_object * loc_obj;
			object_field(*obj, "Location", &loc_obj);
			auto loc = build_point(*loc_obj, s, c);
			std::vector<ifc_interface::ifc_object *> p;
			collection_field(*obj, "P", &p);
			auto xdir = build_direction(*p[0], c);
			auto ydir = build_direction(*p[1], c);
			auto xcol = normalize(xdir.vector());
			auto ycol = normalize(ydir.vector());
			decltype(ycol) zcol(0, 0, 1);
			return result_t(xcol.x(), ycol.x(), zcol.x(), loc.x(),
							xcol.y(), ycol.y(), zcol.y(), loc.y(),
							xcol.z(), ycol.z(), zcol.z(), loc.z());
		}
		else if (is_instance_of(*obj, "IfcAxis2Placement3D")) {
			ifc_interface::ifc_object * loc_obj;
			object_field(*obj, "Location", &loc_obj);
			auto loc = build_point(*loc_obj, s, c);
			std::vector<ifc_interface::ifc_object *> p;
			collection_field(*obj, "P", &p);
			auto xdir = build_direction(*p[0], c);
			auto ydir = build_direction(*p[1], c);
			auto zdir = build_direction(*p[2], c);
			auto xcol = normalize(xdir.vector());
			auto ycol = normalize(ydir.vector());
			auto zcol = normalize(zdir.vector());
			return result_t(xcol.x(), ycol.x(), zcol.x(), loc.x(),
							xcol.y(), ycol.y(), zcol.y(), loc.y(),
							xcol.z(), ycol.z(), zcol.z(), loc.z());
		}
		else if (is_instance_of(*obj, "IfcPlane")) {
			ifc_interface::ifc_object * pos;
			object_field(*obj, "Position", &pos);
			return build_transformation(pos, s, c);
		}
		else if (is_instance_of(*obj, "IfcCartesianTransformationOperator3D")) 
		{
			ifc_interface::ifc_object * loc_obj;
			object_field(*obj, "LocalOrigin", &loc_obj);
			auto loc = build_point(*loc_obj, s, c);
			std::vector<ifc_interface::ifc_object *> p;
			collection_field(*obj, "U", &p);
			double scale;
			real_field(*obj, "Scl", &scale);
			auto xdir = build_direction(*p[0], c);
			auto ydir = build_direction(*p[1], c);
			auto zdir = build_direction(*p[2], c);
			auto xcol = normalize(xdir.vector()) * scale;
			auto ycol = normalize(ydir.vector()) * scale;
			auto zcol = normalize(zdir.vector()) * scale;
			return result_t(xcol.x(), ycol.x(), zcol.x(), loc.x(),
							xcol.y(), ycol.y(), zcol.y(), loc.y(),
							xcol.z(), ycol.z(), zcol.z(), loc.z());
		}
		else {
			// TODO: Throw an exception or something.
			return result_t();
		}
	}
	else {
		return result_t();
	}
}
