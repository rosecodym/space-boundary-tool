#pragma once

#include "precompiled.h"

#include "number_collection.h"
#include "unit_scaler.h"

template <typename KernelT>
CGAL::Point_3<KernelT> build_point(
	const cppw::Instance & inst,
	const unit_scaler & s,
	number_collection<KernelT> * c)
{
	assert(inst.is_kind_of("IfcCartesianPoint"));
	cppw::List coords(inst.get("Coordinates"));
	double x = coords.get_(0);
	double y = coords.get_(1);
	double z = (cppw::Integer)inst.get("Dim") == 3 ? coords.get_(2) : 0.0;
	return c->request_point(s.length_in(x), s.length_in(y), s.length_in(z));
}

template <typename KernelT>
CGAL::Direction_3<KernelT> build_direction(
	const cppw::Instance & inst,
	number_collection<KernelT> * c)
{
	assert(inst.is_instance_of("IfcDirection"));
	cppw::List ratios = inst.get("DirectionRatios");
	double dx = ratios.get_(0);
	double dy = ratios.get_(1);
	double dz = (cppw::Integer)inst.get("Dim") == 3 ? ratios.get_(2) : 0.0;
	return c->request_direction(dx, dy, dz);
}

template <typename KernelT>
CGAL::Aff_transformation_3<KernelT> build_transformation(
	const cppw::Select & sel,
	const unit_scaler & s,
	number_collection<KernelT> * c)
{
	typedef CGAL::Aff_transformation_3<KernelT> result_t;
	if (sel.is_set()) {
		cppw::Instance inst(sel);
		if (inst.is_instance_of("IfcLocalPlacement")) {
			auto relTo = inst.get("PlacementRelTo");
			auto from = inst.get("RelativePlacement");
			return 
				build_transformation(relTo, s, c) *
				build_transformation(from, s, c);
		}
		else if (inst.is_instance_of("IfcAxis2Placement2D")) {
			cppw::Instance location = inst.get("Location");
			auto loc = build_point(location, s, c);
			cppw::Aggregate p = inst.get("P");
			auto xdir = build_direction((cppw::Instance)p.get_(0), c);
			auto ydir = build_direction((cppw::Instance)p.get_(1), c);
			auto xcol = normalize(xdir.vector());
			auto ycol = normalize(ydir.vector());
			decltype(ycol) zcol(0, 0, 1);
			return result_t(xcol.x(), ycol.x(), zcol.x(), loc.x(),
							xcol.y(), ycol.y(), zcol.y(), loc.y(),
							xcol.z(), ycol.z(), zcol.z(), loc.z());
		}
		else if (inst.is_instance_of("IfcAxis2Placement3D")) {
			cppw::Instance location = inst.get("Location");
			auto loc = build_point(location, s, c);
			cppw::Aggregate p = inst.get("P");
			auto xdir = build_direction((cppw::Instance)p.get_(0), c);
			auto ydir = build_direction((cppw::Instance)p.get_(1), c);
			auto zdir = build_direction((cppw::Instance)p.get_(2), c);
			auto xcol = normalize(xdir.vector());
			auto ycol = normalize(ydir.vector());
			auto zcol = normalize(zdir.vector());
			return result_t(xcol.x(), ycol.x(), zcol.x(), loc.x(),
							xcol.y(), ycol.y(), zcol.y(), loc.y(),
							xcol.z(), ycol.z(), zcol.z(), loc.z());
		}
		else if (inst.is_instance_of("IfcPlane")) {
			return build_transformation(inst.get("Position"), s, c);
		}
		else if (inst.is_instance_of("IfcCartesianTransformationOperator3D")) {
			cppw::Instance location = inst.get("LocalOrigin");
			auto loc = build_point(location, s, c);
			cppw::Aggregate p = inst.get("U");
			double scale = (cppw::Real)inst.get("Scl");
			auto xdir = build_direction((cppw::Instance)p.get_(0), c);
			auto ydir = build_direction((cppw::Instance)p.get_(1), c);
			auto zdir = build_direction((cppw::Instance)p.get_(2), c);
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
