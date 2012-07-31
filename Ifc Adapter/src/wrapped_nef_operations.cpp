#include "precompiled.h"

#include "geometry_common.h"
#include "ifc-to-face.h"
#include "sbt-ifcadapter.h"
#include "unit_scaler.h"

#include "wrapped_nef_operations.h"

extern sb_calculation_options g_opts;

namespace {

typedef CGAL::Extended_cartesian<CORE::BigRat>	nef_K;
typedef CGAL::Nef_polyhedron_3<nef_K>			nef_polyhedron_3;
typedef nef_K::Point_3							exact_point_3;
typedef nef_K::Direction_3						exact_direction_3;
typedef nef_K::Vector_3							exact_vector_3;
typedef nef_K::Ray_3							exact_ray_3;
typedef nef_K::Plane_3							exact_plane_3;
typedef nef_K::Aff_transformation_3				exact_transformation_3;

typedef nef_K::FT NT;

class numeric_context {
	
private:

	class interval_wrapper {
	public:

		typedef CGAL::Interval_skip_list_interval<double> inner_interval;
		typedef inner_interval::Value Value;

		inner_interval inner;
		double instanced_low;
		double instanced_high;
		NT actual;

		Value inf() const { return inner.inf(); }
		Value sup() const { return inner.sup(); }
		bool contains(Value v) const { return inner.contains(v); }
		bool contains_interval(Value i, Value s) const { return inner.contains_interval(i, s); }
		bool operator == (const interval_wrapper & rhs) const { return inner == rhs.inner; }
		bool operator != (const interval_wrapper & rhs) const { return inner != rhs.inner; }

		interval_wrapper(double d, double tol) : inner(inner_interval(d - tol, d + tol)), instanced_low(d), instanced_high(d), actual(d) { }
		interval_wrapper(inner_interval inner, double low, double high, NT actual) : inner(inner), instanced_low(low), instanced_high(high), actual(actual) { }

	};
	
	CGAL::Interval_skip_list<interval_wrapper> intervals;
	double tolerance;
	std::map<double, NT> cached;
	NT _zero;
	NT _one;

	std::vector<std::pair<exact_direction_3, exact_vector_3>> directions;

	numeric_context(const numeric_context & src);
	numeric_context & operator = (const numeric_context & src);

public:

	numeric_context(double tol)
		: tolerance(tol)
	{
		_zero = request_coordinate(0.0);
		_one = request_coordinate(1.0);
	}

	const NT & zero() const { return _zero; }
	const NT & one() const { return _one; }

	NT numeric_context::request_coordinate(double d) {

		typedef numeric_context::interval_wrapper::inner_interval interval;
		typedef CGAL::Interval_skip_list<interval> interval_skip_list;

		auto res = cached.find(d);
		if (res != cached.end()) {
			return res->second;
		}

		std::vector<interval_wrapper> ints;
		intervals.find_intervals(d, std::back_inserter(ints));

		if (ints.size() == 0) {
			auto i = interval_wrapper(d, tolerance);
			intervals.insert(i);
			cached[d] = i.actual;
			return i.actual;
		}

		else if (ints.size() == 1) {
			interval_wrapper & i = ints.front();
			intervals.remove(i);
			if (d > i.instanced_high) {
				intervals.insert(interval_wrapper(interval(i.inner.inf(), d + tolerance), i.instanced_low, d, i.actual));
			}
			else if (d < i.instanced_low) {
				intervals.insert(interval_wrapper(interval(d - tolerance, i.inner.sup()), d, i.instanced_high, i.actual));
			}
			else {
				intervals.insert(i);
			}

			cached[d] = i.actual;
			return i.actual;
		}

		else {
			assert(ints.size() == 0 || ints.size() == 1);
			return zero();
		}
	
	}

	exact_point_3 request_point(double x, double y, double z) {
		return exact_point_3(request_coordinate(x), request_coordinate(y), request_coordinate(z));
	}

	exact_direction_3 request_direction(double dx, double dy, double dz) {
		exact_direction_3 d(request_coordinate(dx), request_coordinate(dy), request_coordinate(dz));
		exact_vector_3 v = d.to_vector();
		for (auto p = directions.begin(); p != directions.end(); ++p) {
			if (p->first == d) {
				return d;
			}
			if (CGAL::cross_product(p->second, v).squared_length() == zero()) {
				CGAL::Sign signs_a[] = { CGAL::sign(d.dx()), CGAL::sign(d.dy()), CGAL::sign(d.dz()) };
				CGAL::Sign signs_b[] = { CGAL::sign(p->first.dx()), CGAL::sign(p->first.dy()), CGAL::sign(p->first.dz()) };
				if (signs_a[0] == signs_b[0] &&
					signs_a[1] == signs_b[1] &&
					signs_a[2] == signs_b[2])
				{
					return p->first;
				}
				else {
					return -p->first;
				}
			}
		}
		directions.push_back(std::make_pair(d, v));
		return d;
	}
};

numeric_context nefs_context(0.0005);

exact_point_3 to_exact_point(const cppw::Instance & inst) {
	cppw::List coords = inst.get("Coordinates");
	return nefs_context.request_point((cppw::Real)coords.get_(0), (cppw::Real)coords.get_(1), (cppw::Integer)inst.get("Dim") == 3 ? (cppw::Real)coords.get_(2) : 0);
}

exact_direction_3 to_exact_direction(const cppw::Instance & inst) {
	cppw::List ratios = inst.get("DirectionRatios");
	return nefs_context.request_direction((cppw::Integer)ratios.get_(0), (cppw::Integer)ratios.get_(1), (cppw::Integer)inst.get("Dim") == 3 ? (cppw::Integer)ratios.get_(2) : 0);
}

nef_polyhedron_3 create_nef(const cppw::Instance & inst, const unit_scaler & s) {
	if (inst.is_kind_of("IfcExtrudedAreaSolid")) {
		exact_face base = ifc_to_face((cppw::Instance)inst.get("SweptArea"), s);
		std::vector<exact_point_3> base_points;
		std::vector<exact_point_3> extruded_points;
		std::transform(base.outer_boundary.vertices.begin(), base.outer_boundary.vertices.end(), std::back_inserter(base_points), [](const exact_point & p) {
			return nefs_context.request_point(CGAL::to_double(p.x), CGAL::to_double(p.y), CGAL::to_double(p.z));
		});
		exact_vector_3 extrusion_vec = to_exact_direction((cppw::Instance)inst.get("ExtrudedDirection")).vector();
		nef_K::FT magnitude(nefs_context.request_coordinate(sqrt(CGAL::to_double(extrusion_vec.squared_length()))));
		extrusion_vec = extrusion_vec / magnitude;
		extrusion_vec = extrusion_vec * nefs_context.request_coordinate((cppw::Real)inst.get("Depth"));
		exact_transformation_3 extrusion(CGAL::TRANSLATION, extrusion_vec);
		std::transform(base_points.begin(), base_points.end(), std::back_inserter(extruded_points), [&extrusion](const exact_point_3 & p) {
			return p.transform(extrusion);
		});
		std::vector<exact_plane_3> planes;
		planes.push_back(exact_plane_3(base_points[0], base_points[1], base_points[2]));
		planes.push_back(exact_plane_3(extruded_points[2], extruded_points[1], extruded_points[0]));
		size_t pc = base_points.size();
		for (size_t i = pc; i < pc * 2; ++i) {
			planes.push_back(exact_plane_3(base_points[i % pc], base_points[(i - 1) % pc], extruded_points[(i - 1) % pc]));
		}
		nef_polyhedron_3 result(planes.front().opposite());
		std::for_each(planes.begin(), planes.end(), [&result](const exact_plane_3 & pl) {
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
		exact_direction_3 n = normal.is_set() ? to_exact_direction((cppw::Instance)normal) : exact_direction_3(0, 0, 1);
		exact_plane_3 p(to_exact_point(point), inst.get("AgreementFlag") ? n : -n);
		return nef_polyhedron_3(p);
	}
	else if (inst.is_kind_of("IfcPolygonalBoundedHalfSpace")) {
		exact_face base = ifc_to_face((cppw::Instance)inst.get("PolygonalBoundary"), s);
		std::vector<exact_point_3> base_points;
		std::transform(base.outer_boundary.vertices.begin(), base.outer_boundary.vertices.end(), std::back_inserter(base_points), [](const exact_point & p) {
			return nefs_context.request_point(CGAL::to_double(p.x), CGAL::to_double(p.y), CGAL::to_double(p.z));
		});
		std::vector<exact_plane_3> planes;
		size_t pc = base_points.size();
		for (size_t i = 0; i < pc; ++i) {
			planes.push_back(exact_plane_3(exact_ray_3(base_points[(i + 1) % pc], exact_direction_3(0, 0, 1)), base_points[i]));
		}
		cppw::Instance base_surface = inst.get("BaseSurface");
		if (!base_surface.is_instance_of("IfcPlane")) {
			g_opts.error_func("[Error - tried to use something other than an IfcPlane for the base surface of an IfcPolygonalBoundedHalfSpace.]\n");
			return nef_polyhedron_3();
		}
		cppw::Instance surface_placement = base_surface.get("Position");
		cppw::Instance point = surface_placement.get("Location");
		cppw::Select normal = surface_placement.get("Axis");
		exact_direction_3 n = normal.is_set() ? to_exact_direction((cppw::Instance)normal) : exact_direction_3(0, 0, 1);
		planes.push_back(exact_plane_3(to_exact_point(point), n));
		if (!inst.get("AgreementFlag")) {
			planes.back() = planes.back().opposite();
		}
		nef_polyhedron_3 result(planes.front());
		std::for_each(planes.begin(), planes.end(), [&result](const exact_plane_3 & pl) {
			result *= nef_polyhedron_3(pl);
		});
		return result;
	}
	else if (inst.is_instance_of("IfcBooleanClippingResult")) {
		nef_polyhedron_3 first = create_nef((cppw::Instance)inst.get("FirstOperand"), s);
		nef_polyhedron_3 second = create_nef((cppw::Instance)inst.get("SecondOperand"), s);
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

void convert_to_solid(exact_solid * s, const nef_polyhedron_3 & nef) {
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
				exact_point_3 p = start->source()->center_vertex()->point();
				point_3 req = g_numbers.request_point(
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

void solid_from_boolean_result(exact_solid * s, const cppw::Instance & inst, const unit_scaler & scaler) {
	g_opts.notify_func("(element requires nef processing)...");
	convert_to_solid(s, create_nef(inst, scaler));
	//nef_polyhedron_3 first = create_nef((cppw::Instance)inst.get("FirstOperand"), scaler);
	//nef_polyhedron_3 second = create_nef((cppw::Instance)inst.get("SecondOperand"), scaler);
	//cppw::String op = inst.get("Operator");
	//if (op == "DIFFERENCE") {
	//	convert_to_solid(s, (first - second).regularization());
	//}
	//else if (op == "INTERSECTION") {
	//	convert_to_solid(s, (first * second).regularization());
	//}
	//else if (op == "UNION") {
	//	convert_to_solid(s, (first + second).regularization());
	//}
	//else {
	//	g_opts.error_func("[Aborting - invalid boolean result operator.]\n");
	//	exit(IFCADAPT_UNKNOWN);
	//}
}

} // namespace wrapped_nef_operations