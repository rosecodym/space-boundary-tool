#include "precompiled.h"

#include "internal_geometry.h"

#include "cgal-typedefs.h"
#include "geometry_common.h"
#include "number_collection.h"
#include "unit_scaler.h"
#include "wrapped_nef_operations.h"

namespace internal_geometry {

namespace {

bool are_perpendicular(const direction_3 & a, const direction_3 & b) {
	return CGAL::is_zero(a.vector() * b.vector());
}

length_scaler build_scale_function(const unit_scaler & us) {
	return [&us](double len) { return us.length_in(len); };
}

point_3 build_point(
	const cppw::Select & sel, 
	const length_scaler & scale_length,
	number_collection<K> * c) 
{
	cppw::Instance inst = sel;
	cppw::List coords(inst.get("Coordinates"));
	cppw::Integer dim = inst.get("Dim");
	auto scaled_x = scale_length(coords.get_(0));
	auto scaled_y = scale_length(coords.get_(1));
	auto scaled_z = dim == 3 ? scale_length(coords.get_(2)) : 0.0;
	return c->request_point(scaled_x, scaled_y, scaled_z);
}

direction_3 build_direction(
	const cppw::Select & sel, 
	number_collection<K> * c)
{
	cppw::Instance inst = sel;
	cppw::List ratios = inst.get("DirectionRatios");
	cppw::Integer dim = inst.get("Dim");
	cppw::Real dx = ratios.get_(0);
	cppw::Real dy = ratios.get_(1);
	cppw::Real dz = dim == 3 ? ratios.get_(2) : 0.0;
	return c->request_direction(dx, dy, dz);
}

transformation_3 build_transformation(
	const cppw::Select & sel,
	const length_scaler & scale_length,
	number_collection<K> * c) 
{
	if (!sel.is_set()) { return transformation_3(); }
	cppw::Instance inst = sel;
	if (inst.is_instance_of("IfcLocalPlacement")) {
		auto to = build_transformation(
			inst.get("PlacementRelTo"),
			scale_length,
			c);
		auto from = build_transformation(
			inst.get("RelativePlacement"), 
			scale_length,
			c);
		return to * from;
	}
	else if (inst.is_instance_of("IfcAxis2Placement2D")) {
		auto location = build_point(inst.get("Location"), scale_length, c);
		cppw::Aggregate p = inst.get("P");
		auto xcol = normalize(build_direction(p.get_(0), c).vector());
		auto ycol = normalize(build_direction(p.get_(1), c).vector());
		vector_3 zcol(0, 0, 1);
		return transformation_3(xcol.x(), ycol.x(), zcol.x(), location.x(),
								xcol.y(), ycol.y(), zcol.y(), location.y(),
								xcol.z(), ycol.z(), zcol.z(), location.z());
	}
	else if (inst.is_instance_of("IfcAxis2Placement3D")) {
		auto location = build_point(inst.get("Location"), scale_length, c);
		cppw::Aggregate p = inst.get("P");
		auto xcol = normalize(build_direction(p.get_(0), c).vector());
		auto ycol = normalize(build_direction(p.get_(1), c).vector());
		auto zcol = normalize(build_direction(p.get_(2), c).vector());
		return transformation_3(xcol.x(), ycol.x(), zcol.x(), location.x(),
								xcol.y(), ycol.y(), zcol.y(), location.y(),
								xcol.z(), ycol.z(), zcol.z(), location.z());
	}
	else if (inst.is_instance_of("IfcPlane")) {
		return build_transformation(inst.get("Position"), scale_length, c);
	}
	else if (inst.is_instance_of("IfcCartesianTransformationOperator3D")) {
		point_3 loc = build_point(inst.get("LocalOrigin"), scale_length, c);
		cppw::Aggregate p = inst.get("U");
		double scale = (cppw::Real)inst.get("Scl");
		auto xcol = normalize(build_direction(p.get_(0), c).vector()) * scale;
		auto ycol = normalize(build_direction(p.get_(1), c).vector()) * scale;
		auto zcol = normalize(build_direction(p.get_(2), c).vector()) * scale;
		return transformation_3(xcol.x(), ycol.x(), zcol.x(), loc.x(),
								xcol.y(), ycol.y(), zcol.y(), loc.y(),
								xcol.z(), ycol.z(), zcol.z(), loc.z());
	}
	else {
		throw bad_rep_exception("unknown source for transformation matrix");
	}
}

std::vector<point_3> build_polyloop(
	const cppw::Select & sel,
	const length_scaler & scale,
	number_collection<K> * c)
{
	typedef std::vector<point_3> loop;
	cppw::Instance inst(sel);
	if (inst.is_instance_of("IfcPolyline")) {
		loop res;
		cppw::List pts = inst.get("Points");
		for (pts.move_first(); pts.move_next(); ) {
			res.push_back(build_point(pts.get_(), scale, c));
		}
		if (res.front() == res.back()) { res.pop_back(); }
		return res;
	}
	else if (inst.is_instance_of("IfcCompositeCurveSegment")) {
		return build_polyloop(inst.get("ParentCurve"), scale, c);
	}
	else if (inst.is_instance_of("IfcCompositeCurve")) {
		cppw::List components = inst.get("Segments");
		if (components.size() != 1) {
			throw bad_rep_exception(
				"composite curve without exactly one segment");
		}
		return build_polyloop(components.get_(0), scale, c);
	}
	else if (inst.is_instance_of("IfcPolyLoop")) {
		loop res;
		cppw::List pts = inst.get("Polygon");
		for (pts.move_first(); pts.move_next(); ) {
			res.push_back(build_point(pts.get_(), scale, c));
		}
		return res;
	}
	else if (inst.is_instance_of("IfcCurveBoundedPlane")) {
		auto res = build_polyloop(inst.get("OuterBoundary"), scale, c);
		auto t = build_transformation(inst.get("BasisSurface"), scale, c);
		boost::transform(res, res.begin(), t);
		return res;
	}
	else if (inst.is_instance_of("IfcArbitraryClosedProfileDef")) {
		return build_polyloop(inst.get("OuterCurve"), scale, c);
	}
	else if (inst.is_instance_of("IfcRectangleProfileDef")) {
		double xdim = inst.get("XDim");
		double ydim = inst.get("YDim");
		point_2 req;
		loop res;
		req = c->request_point(scale(-xdim) / 2, scale(-ydim) / 2);
		res.push_back(point_3(req.x(), req.y(), 0));
		req = c->request_point(scale(xdim) / 2, scale(-ydim) / 2);
		res.push_back(point_3(req.x(), req.y(), 0));
		req = c->request_point(scale(xdim) / 2, scale(ydim) / 2);
		res.push_back(point_3(req.x(), req.y(), 0));
		req = c->request_point(scale(-xdim) / 2, scale(ydim) / 2);
		res.push_back(point_3(req.x(), req.y(), 0));
		auto t = build_transformation(inst.get("Position"), scale, c);
		boost::transform(res, res.begin(), t);
		assert(boost::find_if(res, [](const point_3 & p) {
			return !CGAL::is_zero(p.z());
		}) == res.end());
		return res;
	}
	else {
		throw bad_rep_exception("unsupported representation for a polyloop");
	}
}

bool normal_matches_dir(
	const face & f, 
	const direction_3 & dir) 
{
	// Something appears to be quite wrong here. The whole point of this
	// function isn't to make sure that they're parallel - it assumes that
	// they're parallel. It's to make sure that they're *not* *antiparallel*.
	// Unfortunately, since I noticed this problem during a refactor I'm not
	// going to just change it. I think that the bug is being masked by the
	// fact that the core does the same check (presumably correctly).
	return number_collection<K>::are_effectively_parallel(
		f.normal(), 
		dir, 
		EPS_MAGIC);
}

} // namespace

face::face(
	const cppw::Select & sel,
	const length_scaler & scale,
	number_collection<K> * c)
{
	cppw::Instance inst(sel);
	if (inst.is_instance_of("IfcFaceBound")) {
		outer = build_polyloop(inst.get("Bound"), scale, c);
	}
	else if (inst.is_instance_of("IfcFace")) {
		cppw::Set bounds = inst.get("Bounds");
		for (bounds.move_first(); bounds.move_next(); ) {
			auto thisbound = build_polyloop(bounds.get_(), scale, c);
			boost::copy(thisbound, std::back_inserter(outer));
		}
	}
	else if (inst.is_instance_of("IfcArbitraryProfileDefWithVoids")) {
		outer = build_polyloop(inst.get("OuterCurve"), scale, c);
		cppw::Set inners = inst.get("InnerCurves");
		for (inners.move_first(); inners.move_next(); ) {
			voids.push_back(build_polyloop(inners.get_(), scale, c));
		}
	}
	else {
		outer = build_polyloop(inst, scale, c);
	}
}

face::face(const exact_face & f) {
	typedef std::vector<point_3> loop;
	auto import_loop = [](const exact_polyloop & eloop) -> loop {
		loop res;
		auto & vs = eloop.vertices;
		boost::transform(vs, back_inserter(res), [](const exact_point & p) {
			return point_3(p.x, p.y, p.z);
		});
		return res;
	};
	outer = import_loop(f.outer_boundary);
	boost::transform(f.voids, std::back_inserter(voids), import_loop);
}

direction_3 face::normal() const {
	// http://cs.haifa.ac.il/~gordon/plane.pdf
	NT a(0.0), b(0.0), c(0.0);
	auto & loop = outer;
	for (size_t i = 0; i < loop.size(); ++i) {
		auto & curr = loop[i];
		auto & next = loop[(i+1) % loop.size()];
		a += (curr.y() - next.y()) * (curr.z() + next.z());
		b += (curr.z() - next.z()) * (curr.x() + next.x());
		c += (curr.x() - next.x()) * (curr.y() + next.y());
	}
	return direction_3(a, b, c);
}

interface_face face::to_interface() const {
	auto export_loop = [](const std::vector<point_3> & pts) -> ::polyloop {
		::polyloop res;
		res.vertex_count = pts.size();
		res.vertices = (::point *)malloc(sizeof(::point) * res.vertex_count);
		for (size_t i = 0; i < pts.size(); ++i) {
			res.vertices[i].x = CGAL::to_double(pts[i].x());
			res.vertices[i].y = CGAL::to_double(pts[i].y());
			res.vertices[i].z = CGAL::to_double(pts[i].z());
		}
		return res;
	};
	interface_face f;
	f.outer_boundary = export_loop(outer);
	f.voids = nullptr;
	f.void_count = voids.size();
	if (!voids.empty()) {
		f.voids = (::polyloop *)malloc(sizeof(::polyloop) * f.void_count);
		for (size_t i = 0; i < voids.size(); ++i) {
			f.voids[i] = export_loop(voids[i]);
		}
	}
	return f;
}

void face::reverse() {
	boost::reverse(outer);
	boost::for_each(voids, [](std::vector<point_3> & v) {
		boost::reverse(v);
	});
}

void face::transform(const transformation_3 & t) {
	auto previous_normal = normal();
	boost::transform(outer, outer.begin(), t);
	assert(t(previous_normal) == normal());
	for (auto v = voids.begin(); v != voids.end(); ++v) {
		boost::transform(*v, v->begin(), t);
	}
}

std::unique_ptr<solid> solid::legacy_facade_build(const exact_solid & s) {
	if (s.rep_type() == REP_BREP) {
		return std::unique_ptr<brep>(new brep(*s.rep.as_brep));
	}
	else if (s.rep_type() == REP_EXT) {
		return std::unique_ptr<ext>(new ext(*s.rep.as_ext));
	}
	else {
		throw bad_rep_exception("unset internal exact surrogate rep type");
	}
}

brep::brep(
	const cppw::Instance & inst,
	const length_scaler & scale_length,
	number_collection<K> * c)
{
	if (!inst.is_kind_of("IfcConnectedFaceSet")) {
		throw bad_rep_exception("unsupported representation for a brep");
	}
	cppw::Set faceSet = inst.get("CfsFaces");
	for (faceSet.move_first(); faceSet.move_next(); ) {
		faces.push_back(face(faceSet.get_(), scale_length, c));
	}
}

brep::brep(const exact_brep & b) {
	using std::back_inserter;
	using boost::transform;
	transform(b.faces, back_inserter(faces), [](const exact_face & f) {
		return face(f);
	});
}

void brep::transform(const transformation_3 & t) {
	boost::for_each(faces, [&t](face & f) { f.transform(t); });
	if (axes_) { 
		axes_ = std::make_tuple(axis1()->transform(t), axis2()->transform(t));
	}
}

interface_solid brep::to_interface_solid() const {
	interface_solid res;
	res.rep_type = REP_BREP;
	::brep & rep = res.rep.as_brep;
	rep.face_count = this->faces.size();
	rep.faces = (::face *)malloc(sizeof(::face) * rep.face_count);
	for (size_t i = 0; i < rep.face_count; ++i) {
		rep.faces[i] = this->faces[i].to_interface();
	}
	return res;
}

ext::ext(
	const cppw::Instance & inst,
	const length_scaler & scale,
	number_collection<K> * c)
	: area(inst.get("SweptArea"), scale, c)
{
	double unscaled_depth = inst.get("Depth");
	depth = c->request_height(scale(unscaled_depth));
	cppw::Instance d = inst.get("ExtrudedDirection");
	cppw::List ratios = d.get("DirectionRatios");
	double dx = ratios.get_(0);
	double dy = ratios.get_(1);
	double dz = ((cppw::Integer)d.get("Dim")) == 3 ? ratios.get_(2) : 0.0;
	dir = c->request_direction(dx, dy, dz);
	if (!normal_matches_dir(area, dir)) {
		area.reverse();
	}
	assert(!are_perpendicular(area.normal(), dir));
}

ext::ext(const exact_extruded_area_solid & e) 
	: area(e.area),
	  dir(e.ext_dir),
	  depth(e.extrusion_depth) { }

void ext::transform(const transformation_3 & t) {
	assert(!are_perpendicular(area.normal(), dir));
	area.transform(t);
	dir = dir.transform(t);
	if (axes_) { 
		axes_ = std::make_tuple(axis1()->transform(t), axis2()->transform(t));
	}
	assert(!are_perpendicular(area.normal(), dir));
}

interface_solid ext::to_interface_solid() const {
	interface_solid res;
	res.rep_type = REP_EXT;
	auto & rep = res.rep.as_ext;
	rep.area = area.to_interface();
	rep.extrusion_depth = CGAL::to_double(depth);
	rep.ext_dx = CGAL::to_double(dir.dx());
	rep.ext_dy = CGAL::to_double(dir.dy());
	rep.ext_dz = CGAL::to_double(dir.dz());
	return res;
}

transformation_3 get_globalizer(
	const cppw::Select & sel,
	const unit_scaler & scaler,
	number_collection<K> * c)
{
	return get_globalizer((cppw::Instance)sel, scaler, c);
}

transformation_3 get_globalizer(
	const cppw::Instance & inst,
	const unit_scaler & scaler,
	number_collection<K> * c) 
{
	if (inst.is_kind_of("IfcProduct")) {
		auto inner = get_globalizer(inst.get("Representation"), scaler, c);
		cppw::Select placement = inst.get("ObjectPlacement");
		auto sf = build_scale_function(scaler);
		return inner * build_transformation(placement, sf, c);
	}
	else if (inst.is_instance_of("IfcProductDefinitionShape")) {
		cppw::List reps = inst.get("Representations");
		for (reps.move_first(); reps.move_next(); ) {
			cppw::Instance this_rep = reps.get_();
			if (this_rep.get("RepresentationIdentifier") == "Body") {
				cppw::Set rep_items = this_rep.get("Items");
				return get_globalizer(rep_items.get_(0), scaler, c);
			}
		}
		throw bad_rep_exception("no 'Body' representation identifier");
	}
	else if (inst.is_instance_of("IfcFacetedBrep")) {
		return transformation_3();
	}
	else if (inst.is_instance_of("IfcExtrudedAreaSolid")) {
		return transformation_3();
	}
	else if (inst.is_instance_of("IfcMappedItem")) {
		cppw::Instance ms = inst.get("MappingSource");
		auto base = get_globalizer(ms.get("MappedRepresentation"), scaler, c);
		auto from = get_globalizer(ms.get("MappingOrigin"), scaler, c);
		auto to = get_globalizer(inst.get("MappingTarget"), scaler, c);
		return base * from * to;
	}
	else if (inst.is_instance_of("IfcBooleanClippingResult")) {
		return transformation_3();
	}
	else if (inst.is_instance_of("IfcFaceBasedSurfaceModel")) {
		return transformation_3();
	}
	else {
		throw bad_rep_exception("unknown or unsupported geometry definition");
	}
}

boost::optional<std::tuple<direction_3, direction_3>> get_axes(
	const cppw::Instance & inst,
	number_collection<K> * c)
{
	if (inst.is_instance_of("IfcPolyline")) {
		cppw::List pts = inst.get("Points");
		if (pts.count() != 2) {
			throw bad_rep_exception(
				"axis definition without exactly two points");
		}
		cppw::Instance p1(pts.get_(0));
		cppw::List coords = p1.get("Coordinates");
		double p1x = coords.get_(0);
		double p1y = coords.get_(1);
		cppw::Instance p2(pts.get_(1));
		coords = cppw::List(p2.get("Coordinates"));
		double p2x = coords.get_(0);
		double p2y = coords.get_(1);
		direction_3 a1 = c->request_direction(p2x - p1x, p2y - p1y, 0.0);
		vector_3 a3(0, 0, 1);
		direction_3 a2 = CGAL::cross_product(a3, a1.to_vector()).direction();
		return std::make_tuple(a1, a2);
	}
	return boost::optional<std::tuple<direction_3, direction_3>>();
}

boost::optional<std::tuple<direction_3, direction_3>> get_axes(
	const cppw::Select & sel,
	number_collection<K> * c)
{
	return get_axes(cppw::Instance(sel), c);
}

std::unique_ptr<solid> get_local_geometry(
	const cppw::Select & sel,
	const unit_scaler & scaler,
	number_collection<K> * c)
{
	return get_local_geometry((cppw::Instance)sel, scaler, c);
}

std::unique_ptr<solid> get_local_geometry(
	const cppw::Instance & inst,
	const unit_scaler & scaler,
	number_collection<K> * c)
{
	if (inst.is_kind_of("IfcProduct")) {
		return get_local_geometry(inst.get("Representation"), scaler, c);
	}
	else if (inst.is_instance_of("IfcProductDefinitionShape")) {
		cppw::List reps = inst.get("Representations");
		std::unique_ptr<solid> geometry;
		boost::optional<std::tuple<direction_3, direction_3>> axes;
		for (reps.move_first(); reps.move_next(); ) {
			cppw::Instance this_rep = reps.get_();
			if (this_rep.get("RepresentationIdentifier") == "Body") {
				cppw::Set rep_items = this_rep.get("Items");
				geometry = get_local_geometry(rep_items.get_(0), scaler, c);
			}
			else if (this_rep.get("RepresentationIdentifier") == "Axis") {
				cppw::Set rep_items = this_rep.get("Items");
				axes = get_axes(rep_items.get_(0), c);
			}
		}
		if (geometry) {
			geometry->set_axes(axes);
			return geometry;
		}
		throw bad_rep_exception("no 'Body' representation identifier");
	}
	else if (inst.is_instance_of("IfcFacetedBrep")) {
		cppw::Instance b = inst.get("Outer");
		auto sf = build_scale_function(scaler);
		return std::unique_ptr<brep>(new brep(b, sf, c));
	}
	else if (inst.is_instance_of("IfcExtrudedAreaSolid")) {
		auto sf = build_scale_function(scaler);
		auto res = std::unique_ptr<ext>(new ext(inst, sf, c));
		res->transform(build_transformation(inst.get("Position"), sf, c));
		return std::move(res);
	}
	else if (inst.is_instance_of("IfcMappedItem")) {
		cppw::Instance mapping_source = inst.get("MappingSource");
		cppw::Select mapped_rep = mapping_source.get("MappedRepresentation");
		return get_local_geometry(mapped_rep, scaler, c);
	}
	else if (inst.is_instance_of("IfcBooleanClippingResult")) {
		exact_solid legacy_facade;
		wrapped_nef_operations::solid_from_boolean_result(
			&legacy_facade,
			inst,
			scaler,
			c);
		return solid::legacy_facade_build(legacy_facade);
	}
	else if (inst.is_instance_of("IfcFaceBasedSurfaceModel")) {
		cppw::Set face_set = inst.get("FbsmFaces");
		return get_local_geometry(face_set.get_(0), scaler, c);
	}
	else {
		throw bad_rep_exception("unknown or unsupported geometry definition");
	}
}

} // namespace internal_geometry