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

/*
std::tuple<plane_3, point_3> calculate_plane_and_average_point(const loop & l) {
	// http://cs.haifa.ac.il/~gordon/plane.pdf
	NT a(0.0);
	NT b(0.0);
	NT c(0.0);
	NT x(0.0);
	NT y(0.0);
	NT z(0.0);
	for (size_t i = 0; i < l.size(); ++i) {
		const point_3 & curr = l[i];
		const point_3 & next = l[(i+1) % l.size()];
		a += (curr.y() - next.y()) * (curr.z() + next.z());
		b += (curr.z() - next.z()) * (curr.x() + next.x());
		c += (curr.x() - next.x()) * (curr.y() + next.y());
		x += curr.x();
		y += curr.y();
		z += curr.z();
	}
	vector_3 avg_vec(x / l.size(), y / l.size(), z / l.size());
	return std::make_tuple(plane_3(a, b, c, -avg_vec * vector_3(a, b, c)), CGAL::ORIGIN + avg_vec);
}*/

bool normal_matches_extrusion(const exact_face & face, const direction_3 & ext_dir) {
	// http://cs.haifa.ac.il/~gordon/plane.pdf
	NT a(0.0), b(0.0), c(0.0);
	auto & loop = face.outer_boundary.vertices;
	for (size_t i = 0; i < loop.size(); ++i) {
		auto & curr = loop[i];
		auto & next = loop[(i+1) % loop.size()];
		a += (curr.y - next.y) * (curr.z + next.z);
		b += (curr.z - next.z) * (curr.x + next.x);
		c += (curr.x - next.x) * (curr.y + next.y);
	}
	return number_collection::are_effectively_parallel(direction_3(a, b, c), ext_dir, g_opts.equality_tolerance);
}