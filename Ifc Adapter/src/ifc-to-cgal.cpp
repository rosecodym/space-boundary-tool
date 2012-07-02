#include "precompiled.h"

#include "geometry_common.h"
#include "sbt-ifcadapter.h"
#include "unit_scaler.h"

#include "ifc-to-cgal.h"

extern sb_calculation_options g_opts;

point_3 build_point(const cppw::Instance & inst, const unit_scaler & s) {
	assert(inst.is_kind_of("IfcCartesianPoint"));
	cppw::List coords(inst.get("Coordinates"));
	if ((cppw::Integer)inst.get("Dim") == 3) {
		return g_numbers.request_point(s.length_in(coords.get_(0)), s.length_in(coords.get_(1)), s.length_in(coords.get_(2)));
	}
	else {
		return g_numbers.request_point(s.length_in(coords.get_(0)), s.length_in(coords.get_(1)), 0);
	}
}

direction_3 build_direction(const cppw::Instance & inst) {
	assert(inst.is_instance_of("IfcDirection"));
	cppw::List ratios = inst.get("DirectionRatios");
	if ((cppw::Integer)inst.get("Dim") == 3) {
		return g_numbers.request_direction(ratios.get_(0), ratios.get_(1), ratios.get_(2));
	}
	else {
		return g_numbers.request_direction(ratios.get_(0), ratios.get_(1), 0);
	}
}

transformation_3 build_transformation(const cppw::Select & sel, const unit_scaler & s) {
	if (sel.is_set()) {
		cppw::Instance inst(sel);
		if (inst.is_instance_of("IfcLocalPlacement")) {
			return build_transformation(inst.get("PlacementRelTo"), s) * build_transformation(inst.get("RelativePlacement"), s);
		}
		else if (inst.is_instance_of("IfcAxis2Placement2D")) {
			point_3 location = build_point((cppw::Instance)inst.get("Location"), s);
			cppw::Aggregate p = inst.get("P");
			auto xcol = normalize(build_direction((cppw::Instance)p.get_(0)).vector());
			auto ycol = normalize(build_direction((cppw::Instance)p.get_(1)).vector());
			vector_3 zcol(0, 0, 1);
			return transformation_3(xcol.x(), ycol.x(), zcol.x(), location.x(),
									xcol.y(), ycol.y(), zcol.y(), location.y(),
									xcol.z(), ycol.z(), zcol.z(), location.z());
		}
		else if (inst.is_instance_of("IfcAxis2Placement3D")) {
			point_3 location = build_point((cppw::Instance)inst.get("Location"), s);
			cppw::Aggregate p = inst.get("P");
			auto xcol = normalize(build_direction((cppw::Instance)p.get_(0)).vector());
			auto ycol = normalize(build_direction((cppw::Instance)p.get_(1)).vector());
			auto zcol = normalize(build_direction((cppw::Instance)p.get_(2)).vector());
			return transformation_3(xcol.x(), ycol.x(), zcol.x(), location.x(),
									xcol.y(), ycol.y(), zcol.y(), location.y(),
									xcol.z(), ycol.z(), zcol.z(), location.z());
		}
		else if (inst.is_instance_of("IfcPlane")) {
			return build_transformation(inst.get("Position"), s);
		}
		else if (inst.is_instance_of("IfcCartesianTransformationOperator3D")) {
			point_3 location = build_point((cppw::Instance)inst.get("LocalOrigin"), s);
			cppw::Aggregate p = inst.get("U");
			double scale = (cppw::Real)inst.get("Scl");
			auto xcol = normalize(build_direction((cppw::Instance)p.get_(0)).vector()) * scale;
			auto ycol = normalize(build_direction((cppw::Instance)p.get_(1)).vector()) * scale;
			auto zcol = normalize(build_direction((cppw::Instance)p.get_(2)).vector()) * scale;
			return transformation_3(xcol.x(), ycol.x(), zcol.x(), location.x(),
									xcol.y(), ycol.y(), zcol.y(), location.y(),
									xcol.z(), ycol.z(), zcol.z(), location.z());
		}
		else {
			g_opts.error_func("[Unknown source for transformation matrix.]\n");
			return transformation_3();
		}
	}
	else {
		return transformation_3();
	}
}