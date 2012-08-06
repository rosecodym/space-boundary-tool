#include "precompiled.h"

#include "cgal-typedefs.h"
#include "geometry_common.h"
#include "ifc-to-cgal.h"
#include "number_collection.h"
#include "unit_scaler.h"
#include "util.h"

#include "ifc-to-face.h"

extern bool found_freestanding;
extern sb_calculation_options g_opts;

namespace {

exact_polyloop ifc_to_polyloop(const cppw::Instance & inst, const unit_scaler & s, number_collection * c) {
	if (inst.is_instance_of("IfcPolyline")) {
		exact_polyloop loop;
		cppw::List raw_points = inst.get("Points");
		for (raw_points.move_first(); raw_points.move_next(); ) {
			point_3 p = build_point((cppw::Instance)raw_points.get_(), s, c);
			loop.vertices.push_back(exact_point(p.x(), p.y(), p.z()));
		}
		if (loop.vertices.front() == loop.vertices.back()) {
			loop.vertices.pop_back();
		}
		return loop;
	}
	g_opts.error_func("[Fell off the end of a polyloop construction.]\n");
	return exact_polyloop();
}

void transform_according_to(exact_polyloop * p, const transformation_3 & t) {
	std::transform(p->vertices.begin(), p->vertices.end(), p->vertices.begin(), [&t](const exact_point & pt) -> exact_point {
		point_3 transed = point_3(pt.x, pt.y, pt.z).transform(t);
		return exact_point(transed.x(), transed.y(), transed.z());
	});
}

direction_3 find_normal(const std::vector<point_3> & points) {
	assert(points.size() >= 3);
	for (size_t i = 2; i < points.size(); ++i) {
		plane_3 pl(points[0], points[1], points[i]);
		if (!pl.is_degenerate()) {
			return pl.orthogonal_direction();
		}
	}
	g_opts.error_func("[Tried to find the normal of a line.]\n");
	return direction_3();
}

} // namespace

void transform_according_to(exact_face * f, const cppw::Select & trans, const unit_scaler & s, number_collection * c) {
	transformation_3 t = build_transformation(trans, s, c);
	transform_according_to(&f->outer_boundary, t);
	std::for_each(f->voids.begin(), f->voids.end(), [&t](exact_polyloop & p) {
		transform_according_to(&p, t);
	});
}

exact_face ifc_to_face(const cppw::Instance & inst, const unit_scaler & s, number_collection * c) {
	if (inst.is_instance_of("IfcPolyline")) {
		exact_face f;

		cppw::List raw_points = inst.get("Points");
		for (raw_points.move_first(); raw_points.move_next(); ) {
			point_3 p = build_point((cppw::Instance)raw_points.get_(), s, c);
			f.outer_boundary.vertices.push_back(exact_point(p.x(), p.y(), p.z()));
		}
		if (f.outer_boundary.vertices.front() == f.outer_boundary.vertices.back()) {
			f.outer_boundary.vertices.pop_back();
		}
		return f;
	}
	else if (inst.is_instance_of("IfcCompositeCurveSegment")) {
		return ifc_to_face((cppw::Instance)inst.get("ParentCurve"), s, c);
	}
	else if (inst.is_instance_of("IfcCompositeCurve")) {
		cppw::List components = inst.get("Segments");
		assert(components.size() != 1);
		return ifc_to_face((cppw::Instance)components.get_(0), s, c);
	}
	else if (inst.is_instance_of("IfcPolyLoop")) {
		exact_face face;
		cppw::List raw_points = inst.get("Polygon");
		for (raw_points.move_first(); raw_points.move_next(); ) {
			point_3 p = build_point((cppw::Instance)raw_points.get_(), s, c);
			face.outer_boundary.vertices.push_back(exact_point(p.x(), p.y(), p.z()));
		}
		return face;
	}
	else if (inst.is_kind_of("IfcFaceBound")) {
		return ifc_to_face((cppw::Instance)inst.get("Bound"), s, c);
	}
	else if (inst.is_instance_of("IfcFace")) {
		exact_face f;
		cppw::Set bounds = inst.get("Bounds");
		for (bounds.move_first(); bounds.move_next(); ) {
			exact_face thisbound = ifc_to_face((cppw::Instance)bounds.get_(), s, c);
			std::copy(thisbound.outer_boundary.vertices.begin(), thisbound.outer_boundary.vertices.end(), std::back_inserter(f.outer_boundary.vertices));
		}
		return f;
	}
	else if (inst.is_instance_of("IfcCurveBoundedPlane")) {
		exact_face face = ifc_to_face((cppw::Instance)inst.get("OuterBoundary"), s, c);
		transform_according_to(&face, inst.get("BasisSurface"), s, c);
		return face;
	}
	else if (inst.is_instance_of("IfcArbitraryClosedProfileDef")) {
		return ifc_to_face((cppw::Instance)inst.get("OuterCurve"), s, c);
	}
	else if (inst.is_instance_of("IfcRectangleProfileDef")) {
		double xdim = inst.get("XDim");
		double ydim = inst.get("YDim");
		exact_face f;
		point_2 req;
		req = c->request_point(s.length_in(-xdim) / 2, s.length_in(-ydim) / 2);
		f.outer_boundary.vertices.push_back(exact_point(req.x(), req.y(), 0));
		req = c->request_point(s.length_in(xdim) / 2, s.length_in(-ydim) / 2);
		f.outer_boundary.vertices.push_back(exact_point(req.x(), req.y(), 0));
		req = c->request_point(s.length_in(xdim) / 2, s.length_in(ydim) / 2);
		f.outer_boundary.vertices.push_back(exact_point(req.x(), req.y(), 0));
		req = c->request_point(s.length_in(-xdim) / 2, s.length_in(ydim) / 2);
		f.outer_boundary.vertices.push_back(exact_point(req.x(), req.y(), 0));
		transform_according_to(&f, inst.get("Position"), s, c);
		return f;
	}
	else if (inst.is_instance_of("IfcArbitraryProfileDefWithVoids")) {
		found_freestanding = true;
		exact_face f;
		f.outer_boundary = ifc_to_polyloop((cppw::Instance)inst.get("OuterCurve"), s, c);
		cppw::Set innerCurves = inst.get("InnerCurves");
		for (innerCurves.move_first(); innerCurves.move_next(); ) {
			f.voids.push_back(ifc_to_polyloop((cppw::Instance)innerCurves.get_(), s, c));
		}
		return f;
	}
	else {
		assert(false);
		return exact_face();
	}
}

bool normal_matches_extrusion(const exact_face & face, const direction_3 & ext_dir) {

	std::vector<point_3> points_3;
	std::transform(face.outer_boundary.vertices.begin(), face.outer_boundary.vertices.end(), std::back_inserter(points_3), [](const exact_point & p) {
		return point_3(p.x, p.y, p.z);
	});

	direction_3 points_dir = find_normal(points_3);
	
	assert(ext_dir == points_dir || ext_dir == -points_dir);

	if (face.outer_boundary.vertices.size() == 3) {
		return points_dir == ext_dir;
	}

	transformation_3 flatten = build_flatten(points_dir);

	polygon_2 points_2;
	for (auto p = points_3.begin(); p != points_3.end(); ++p) {
		point_3 flat = p->transform(flatten);
		points_2.push_back(point_2(flat.x(), flat.y()));
	}

	if ((CGAL::orientation(points_2[0], points_2[1], points_2[2]) == CGAL::LEFT_TURN) != points_2.is_counterclockwise_oriented()) {
		points_dir = -points_dir;
	}

	return points_dir == ext_dir;
}