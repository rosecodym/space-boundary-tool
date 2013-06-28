#include "precompiled.h"

#include "geometry_common.h"
#include "internal_geometry.h"
#include "number_collection.h"
#include "sbt-ifcadapter.h"
#include "unit_scaler.h"

#include "wrapped_nef_operations.h"

extern sb_calculation_options g_opts;

namespace {

typedef CGAL::Extended_cartesian<leda_rational>	eK;
typedef eK::Point_3								extended_point_3;
typedef eK::Plane_3								extended_plane_3;
typedef CGAL::Polyhedron_3<eK>					extended_polyhedron_3;
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

nef_polyhedron_3 create_nef(
	const cppw::Instance & inst, 
	const unit_scaler & scaler, 
	eqc * c, 
	number_collection<eK> * ec) 
{
	auto s = [&scaler](double x) { return scaler.length_in(x); };
	if (inst.is_kind_of("IfcExtrudedAreaSolid")) {
		internal_geometry::ext internal_ext(inst, s, c);
		using boost::transform;
		using std::back_inserter;
		const auto & bverts = internal_ext.base().outer_boundary();
		std::vector<point_3> points;
		transform(bverts, back_inserter(points), [c](const point_3 & p) {
			return c->request_point(
				CGAL::to_double(p.x()),
				CGAL::to_double(p.y()),
				CGAL::to_double(p.z()));
		});
		cppw::Instance ifc_dir = inst.get("ExtrudedDirection");
		auto extrusion_vec = to_exact_direction(ifc_dir, c).to_vector();
		extrusion_vec = extrusion_vec / sqrt(extrusion_vec.squared_length());
		double depth = inst.get("Depth");
		extrusion_vec = extrusion_vec * c->request_height(depth);
		transformation_3 extrusion(CGAL::TRANSLATION, extrusion_vec);
		points.reserve(bverts.size() * 2);
		transform(points, back_inserter(points), extrusion);
		typedef std::deque<size_t> ixdq;
		std::vector<ixdq> facets;
		ixdq base_indices;
		ixdq target_indices;
		facets.resize(bverts.size());
		for (size_t i = 0; i < bverts.size(); ++i) {
			base_indices.push_back(i);
			target_indices.push_back(i + bverts.size());
			facets[i].push_back(target_indices[i]);
			facets[i].push_back(base_indices[i]);
			facets[(i + 1) % bverts.size()].push_front(target_indices[i]);
			facets[(i + 1) % bverts.size()].push_front(base_indices[i]);
		}
		facets.push_back(base_indices);
		facets.push_back(ixdq(target_indices.rbegin(), target_indices.rend()));
		typedef extended_polyhedron_3::HDS hds_t;
		struct builder : public CGAL::Modifier_base<hds_t> {
			std::vector<extended_point_3> pts_;
			const std::vector<ixdq> & facets_;
			builder(
				const std::vector<point_3> & pts,
				const std::vector<ixdq> & facets,
				number_collection<eK> * ec)
				: facets_(facets) 
			{
				for (auto p = pts.begin(); p != pts.end(); ++p) {
					pts_.push_back(ec->request_point(
						CGAL::to_double(p->x()),
						CGAL::to_double(p->y()),
						CGAL::to_double(p->z())));
				}
			}
			void operator () (hds_t & hds) {
				CGAL::Polyhedron_incremental_builder_3<hds_t> b(hds, true);
				b.begin_surface(pts_.size(), facets_.size());
				for (auto p = pts_.begin(); p != pts_.end(); ++p) {
					b.add_vertex(*p);
				}
				for (auto f = facets_.begin(); f != facets_.end(); ++f) {
					b.add_facet(f->begin(), f->end());
				}
				b.end_surface();
			}
		};
		builder b(points, facets, ec);
		extended_polyhedron_3 poly;
		poly.delegate(b);
		return nef_polyhedron_3(poly);
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
		internal_geometry::face base(inst.get("PolygonalBoundary"), s, c);
		const auto & base_points = base.outer_boundary();
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
		cppw::Instance operand1 = inst.get("FirstOperand");
		cppw::Instance operand2 = inst.get("SecondOperand");
		nef_polyhedron_3 first = create_nef(operand1, scaler, c, ec);
		nef_polyhedron_3 second = create_nef(operand2, scaler, c, ec);
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

std::unique_ptr<internal_geometry::solid> convert_to_solid(
	const nef_polyhedron_3 & nef, 
	eqc * c) 
{
	std::vector<std::vector<point_3>> all_pts;
	nef_polyhedron_3::Halffacet_const_iterator facet;
	int face_index = 0;
	CGAL_forall_halffacets(facet, nef) {
		if (facet->incident_volume()->mark()) {
			if (std::distance(facet->facet_cycles_begin(), facet->facet_cycles_end()) != 1) {
				char buf[256];
				sprintf(buf, "[Error - boolean result solid facet with a hole (%u cycles).]\n", std::distance(facet->facet_cycles_begin(), facet->facet_cycles_end()));
				g_opts.error_func(buf);
				return std::unique_ptr<internal_geometry::solid>();
			}
			auto cycle = facet->facet_cycles_begin();
			all_pts.push_back(std::vector<point_3>());
			nef_polyhedron_3::SHalfedge_around_facet_const_circulator start(cycle);
			nef_polyhedron_3::SHalfedge_around_facet_const_circulator end(cycle);
			CGAL_For_all(start, end) {
				auto p = start->source()->center_vertex()->point();
				all_pts.back().push_back(c->request_point(
					CGAL::to_double(p.x()),
					CGAL::to_double(p.y()),
					CGAL::to_double(p.z())));
			}
			++face_index;
		}
	}
	std::vector<internal_geometry::face> faces;
	for (auto loop = all_pts.begin(); loop != all_pts.end(); ++loop) {
		faces.push_back(internal_geometry::face(*loop));
	}
	auto b = new internal_geometry::brep(faces);
	return std::unique_ptr<internal_geometry::solid>(b);
}

} // namespace

namespace wrapped_nef_operations {

std::unique_ptr<internal_geometry::solid> from_boolean_result(
	const cppw::Instance & inst, 
	const unit_scaler & scaler, eqc * c) 
{
	g_opts.notify_func("(geometry requires boolean operations)...");
	number_collection<eK> extended_context(EPS_MAGIC / 20);
	return convert_to_solid(create_nef(inst, scaler, c, &extended_context), c);
}

} // namespace wrapped_nef_operations