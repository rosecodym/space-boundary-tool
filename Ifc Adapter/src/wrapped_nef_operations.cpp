#include "precompiled.h"

#include "geometry_common.h"
#include "ifc-to-face.h"
#include "number_collection.h"
#include "sbt-ifcadapter.h"
#include "unit_scaler.h"

#include "wrapped_nef_operations.h"

extern sb_calculation_options g_opts;

namespace {

typedef CGAL::Extended_cartesian<leda_rational>	eK;
typedef eK::Point_3								extended_point_3;
typedef eK::Plane_3								extended_plane_3;
typedef CGAL::Nef_polyhedron_3<eK>				nef_polyhedron_3;

typedef number_collection<K> eqc;

point_3 to_exact_point(const cppw::Instance & inst, eqc * c) {
	cppw::List coords = inst.get("Coordinates");
	return c->request_point((cppw::Real)coords.get_(0), (cppw::Real)coords.get_(1), (cppw::Integer)inst.get("Dim") == 3 ? (cppw::Real)coords.get_(2) : 0);
}

direction_3 to_exact_direction(const cppw::Instance & inst, eqc * c) {
	cppw::List ratios = inst.get("DirectionRatios");
	cppw::Real dx = ratios.get_(0);
	cppw::Real dy = ratios.get_(1);
	cppw::Real dz = 0.0;
	if ((cppw::Integer)inst.get("Dim") == 3) { dz = ratios.get_(2); }
	assert(!(dx == 0 && dy == 0 && dz == 0));
	return c->request_direction(dx, dy, dz);
}

extended_plane_3 create_extended_plane(const point_3 & p1, const point_3 & p2, const point_3 & p3, number_collection<eK> * c) {
	return extended_plane_3(
		c->request_point(CGAL::to_double(p1.x()), CGAL::to_double(p1.y()), CGAL::to_double(p1.z())),
		c->request_point(CGAL::to_double(p2.x()), CGAL::to_double(p2.y()), CGAL::to_double(p2.z())),
		c->request_point(CGAL::to_double(p3.x()), CGAL::to_double(p3.y()), CGAL::to_double(p3.z())));
}

extended_plane_3 create_extended_plane(const point_3 & p, const direction_3 & d, number_collection<eK> * c) {
	return extended_plane_3(
		c->request_point(CGAL::to_double(p.x()), CGAL::to_double(p.y()), CGAL::to_double(p.z())),
		c->request_direction(CGAL::to_double(d.dx()), CGAL::to_double(d.dy()), CGAL::to_double(d.dz())));
}

extended_plane_3 create_extended_plane(const ray_3 & r, const point_3 & p, number_collection<eK> * c) {
	return create_extended_plane(p, r.direction(), c);
}

nef_polyhedron_3 create_nef(const cppw::Instance & inst, const unit_scaler & s, eqc * c, number_collection<eK> * ec) {
	if (inst.is_kind_of("IfcExtrudedAreaSolid")) {
		exact_face base = ifc_to_face((cppw::Instance)inst.get("SweptArea"), s, c);
		std::vector<point_3> base_points;
		std::vector<point_3> extruded_points;
		std::transform(base.outer_boundary.vertices.begin(), base.outer_boundary.vertices.end(), std::back_inserter(base_points), [c](const exact_point & p) {
			return c->request_point(CGAL::to_double(p.x), CGAL::to_double(p.y), CGAL::to_double(p.z));
		});
		vector_3 extrusion_vec = to_exact_direction((cppw::Instance)inst.get("ExtrudedDirection"), c).vector();
		extrusion_vec = extrusion_vec / CGAL::sqrt(extrusion_vec.squared_length());
		extrusion_vec = extrusion_vec * c->request_height((cppw::Real)inst.get("Depth"));
		transformation_3 extrusion(CGAL::TRANSLATION, extrusion_vec);
		std::transform(base_points.begin(), base_points.end(), std::back_inserter(extruded_points), [&extrusion](const point_3 & p) {
			return p.transform(extrusion);
		});
		std::vector<extended_plane_3> planes;
		planes.push_back(create_extended_plane(base_points[0], base_points[1], base_points[2], ec));
		planes.push_back(create_extended_plane(extruded_points[2], extruded_points[1], extruded_points[0], ec));
		size_t pc = base_points.size();
		for (size_t i = pc; i < pc * 2; ++i) {
			planes.push_back(create_extended_plane(base_points[i % pc], base_points[(i - 1) % pc], extruded_points[(i - 1) % pc], ec));
		}
		nef_polyhedron_3 result(planes.front().opposite());
		std::for_each(planes.begin(), planes.end(), [&result](const extended_plane_3 & pl) {
			result *= nef_polyhedron_3(pl.opposite());
		});
		return result;
	}
	else if (inst.is_instance_of("IfcHalfspaceSolid") || inst.is_instance_of("IfcBoxedHalfSpace")) {
		cppw::Instance base_surface = inst.get("BaseSurface");
		if (!base_surface.is_instance_of("IfcPlane")) {
			g_opts.error_func("[Error - tried to use something other than an IfcPlane for the base surface of an IfcHalfspaceSolid.]\n");
			return nef_polyhedron_3();
		}
		cppw::Instance surface_placement = base_surface.get("Position");
		cppw::Instance point = surface_placement.get("Location");
		cppw::Select normal = surface_placement.get("Axis");
		direction_3 n = normal.is_set() ? to_exact_direction((cppw::Instance)normal, c) : direction_3(0, 0, 1);
		extended_plane_3 p = create_extended_plane(to_exact_point(point, c), inst.get("AgreementFlag") ? n : -n, ec);
		return nef_polyhedron_3(p);
	}
	else if (inst.is_kind_of("IfcPolygonalBoundedHalfSpace")) {
		exact_face base = ifc_to_face((cppw::Instance)inst.get("PolygonalBoundary"), s, c);
		std::vector<point_3> base_points;
		std::transform(base.outer_boundary.vertices.begin(), base.outer_boundary.vertices.end(), std::back_inserter(base_points), [c](const exact_point & p) {
			return c->request_point(CGAL::to_double(p.x), CGAL::to_double(p.y), CGAL::to_double(p.z));
		});
		std::vector<extended_plane_3> planes;
		size_t pc = base_points.size();
		for (size_t i = 0; i < pc; ++i) {
			planes.push_back(create_extended_plane(ray_3(base_points[(i + 1) % pc], direction_3(0, 0, 1)), base_points[i], ec));
		}
		cppw::Instance base_surface = inst.get("BaseSurface");
		if (!base_surface.is_instance_of("IfcPlane")) {
			g_opts.error_func("[Error - tried to use something other than an IfcPlane for the base surface of an IfcPolygonalBoundedHalfSpace.]\n");
			return nef_polyhedron_3();
		}
		cppw::Instance surface_placement = base_surface.get("Position");
		cppw::Instance point = surface_placement.get("Location");
		cppw::Select normal = surface_placement.get("Axis");
		direction_3 n = normal.is_set() ? to_exact_direction((cppw::Instance)normal, c) : direction_3(0, 0, 1);
		planes.push_back(create_extended_plane(to_exact_point(point, c), n, ec));
		if (!inst.get("AgreementFlag")) {
			planes.back() = planes.back().opposite();
		}
		nef_polyhedron_3 result(planes.front());
		std::for_each(planes.begin(), planes.end(), [&result](const extended_plane_3 & pl) {
			result *= nef_polyhedron_3(pl);
		});
		return result;
	}
	else if (inst.is_instance_of("IfcBooleanClippingResult")) {
		nef_polyhedron_3 first = create_nef((cppw::Instance)inst.get("FirstOperand"), s, c, ec);
		nef_polyhedron_3 second = create_nef((cppw::Instance)inst.get("SecondOperand"), s, c, ec);
		cppw::String op = inst.get("Operator");
		if (op == "DIFFERENCE") {
			return (first - second).regularization();
		}
		else if (op == "INTERSECTION") {
			return (first * second).regularization();
		}
		else if (op == "UNION") {
			return (first + second).regularization();
		}
		else {
			g_opts.error_func("[Error - invalid boolean result operator.]\n");
			return nef_polyhedron_3();
		}
	}
	else {
		g_opts.error_func("[Error - can only create adapter nef from an extruded area solid or a halfspace solid.]\n");
		return nef_polyhedron_3();
	}
}

void convert_to_solid(exact_solid * s, const nef_polyhedron_3 & nef, eqc * c) {
	s->set_rep_type(REP_BREP);
	nef_polyhedron_3::Halffacet_const_iterator facet;
	int face_index = 0;
	CGAL_forall_halffacets(facet, nef) {
		if (facet->incident_volume()->mark()) {
			if (std::distance(facet->facet_cycles_begin(), facet->facet_cycles_end()) != 1) {
				char buf[256];
				sprintf(buf, "[Error - boolean result solid facet with a hole (%u cycles).]\n", std::distance(facet->facet_cycles_begin(), facet->facet_cycles_end()));
				g_opts.error_func(buf);
				return;
			}
			auto cycle = facet->facet_cycles_begin();
			s->rep.as_brep->faces.push_back(exact_face());
			nef_polyhedron_3::SHalfedge_around_facet_const_circulator start(cycle);
			nef_polyhedron_3::SHalfedge_around_facet_const_circulator end(cycle);
			CGAL_For_all(start, end) {
				auto p = start->source()->center_vertex()->point();
				point_3 req = c->request_point(
					CGAL::to_double(p.x()),
					CGAL::to_double(p.y()),
					CGAL::to_double(p.z()));
				s->rep.as_brep->faces.back().outer_boundary.vertices.push_back(exact_point(req.x(), req.y(), req.z()));
			}
			++face_index;
		}
	}
}

} // namespace

namespace wrapped_nef_operations {

void solid_from_boolean_result(exact_solid * s, const cppw::Instance & inst, const unit_scaler & scaler, eqc * c) {
	g_opts.notify_func("(geometry requires boolean operations)...");
	number_collection<eK> extended_context(EPS_MAGIC / 20);
	convert_to_solid(s, create_nef(inst, scaler, c, &extended_context), c);
}

} // namespace wrapped_nef_operations