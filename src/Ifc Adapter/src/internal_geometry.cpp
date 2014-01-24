#include "precompiled.h"

#include "../../Edm Wrapper/edm_wrapper_native_interface.h"

#include "internal_geometry.h"

#include "approximated_curve.h"
#include "build_polyloop.h"
#include "geometry_common.h"
#include "ifc-to-cgal.h"
#include "number_collection.h"
#include "unit_scaler.h"
#include "wrapped_nef_operations.h"

using namespace ifc_interface;

namespace internal_geometry {

namespace {

bool are_perpendicular(const direction_3 & a, const direction_3 & b) {
	return CGAL::is_zero(a.vector() * b.vector());
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

face create_face(
	const ifc_object & obj,
	const unit_scaler & scale,
	number_collection<K> * c)
{
	ifc_object * area_obj;
	object_field(obj, "SweptArea", &area_obj);
	return face(*area_obj, scale, c);
}

std::string rep_id(const ifc_object & obj) {
	std::string res;
	string_field(obj, "RepresentationIdentifier", &res);
	return res;
}

} // namespace

face::face(
	const ifc_object & obj,
	const unit_scaler & scale,
	number_collection<K> * c)
{
	if (is_instance_of(obj, "IfcFaceBound")) {
		ifc_object * bound;
		object_field(obj, "Bound", &bound);
		std::tie(outer_, approximations_) = build_polyloop(*bound, scale, c);
	}
	else if (is_instance_of(obj, "IfcFace")) {
		std::vector<ifc_object *> bounds;
		collection_field(obj, "Bounds", &bounds);
		for (auto b = bounds.begin(); b != bounds.end(); ++b) {
			std::vector<approximated_curve> approxes;
			if (is_instance_of(**b, "IfcFaceOuterBound")) {
				std::tie(outer_, approxes) = build_polyloop(**b, scale, c);
				boost::copy(approxes, std::back_inserter(approximations_));
			}
			else { 
				std::vector<point_3> v;
				std::tie(v, approxes) = build_polyloop(**b, scale, c);
				voids_.push_back(v);
				boost::copy(approxes, std::back_inserter(approximations_));
			}
		}
	}
	else if (is_instance_of(obj, "IfcArbitraryProfileDefWithVoids")) {
		ifc_object * outer;
		object_field(obj, "OuterCurve", &outer);
		std::tie(outer_, approximations_) = build_polyloop(*outer, scale, c);
		std::vector<ifc_object *> inners;
		collection_field(obj, "InnerCurves", &inners);
		for (auto p = inners.begin(); p != inners.end(); ++p) {
			std::vector<point_3> v;
			std::vector<approximated_curve> approxes;
			std::tie(v, approxes) = build_polyloop(**p, scale, c);
			voids_.push_back(v);
			boost::copy(approxes, std::back_inserter(approximations_));
		}
	}
	else {
		std::tie(outer_, approximations_) = build_polyloop(obj, scale, c);
	}
}

const std::vector<approximated_curve> & face::approximations() const {
	return approximations_;
}

direction_3 face::normal() const {
	// http://cs.haifa.ac.il/~gordon/plane.pdf
	NT a(0.0), b(0.0), c(0.0);
	auto & loop = outer_;
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
	f.outer_boundary = export_loop(outer_);
	f.voids = nullptr;
	f.void_count = voids_.size();
	if (!voids_.empty()) {
		f.voids = (::polyloop *)malloc(sizeof(::polyloop) * f.void_count);
		for (size_t i = 0; i < voids_.size(); ++i) {
			f.voids[i] = export_loop(voids_[i]);
		}
	}
	return f;
}

void face::reverse() {
	boost::reverse(outer_);
	boost::for_each(voids_, [](std::vector<point_3> & v) {
		boost::reverse(v);
	});
}

void face::transform(const transformation_3 & t) {
	auto previous_normal = normal();
	boost::transform(outer_, outer_.begin(), t);
	// Note that t(previous_normal) does not equal normal() here if the columns
	// of the transformation matrix are not orthnormal, which can happen due to
	// numeric instability in the IfcBuildAxes function. This isn't really
	// worth "fixing" because IfcBuildAxes keeps the columns very *close* to
	// orthonormal.
	for (auto v = voids_.begin(); v != voids_.end(); ++v) {
		boost::transform(*v, v->begin(), t);
	}
	for (auto a = approximations_.begin(); a != approximations_.end(); ++a) {
		*a = a->transformed(t);
	}
}

brep::brep(
	const ifc_object & obj,
	const unit_scaler & scale_length,
	number_collection<K> * c)
{
	if (!is_kind_of(obj, "IfcConnectedFaceSet")) {
		throw bad_rep_exception("unsupported representation for a brep");
	}
	std::vector<ifc_object *> face_set;
	collection_field(obj, "CfsFaces", &face_set);
	for (auto f = face_set.begin(); f != face_set.end(); ++f) {
		faces.push_back(face(**f, scale_length, c));
	}
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

std::vector<approximated_curve> brep::approximations() const {
	std::vector<approximated_curve> res;
	for (auto f = faces.begin(); f != faces.end(); ++f) {
		boost::copy(f->approximations(), std::back_inserter(res));
	}
	return res;
}

ext::ext(
	const ifc_object & obj,
	const unit_scaler & scale,
	number_collection<K> * c)
	: area_(create_face(obj, scale, c))
{
	double unscaled_depth;
	real_field(obj, "Depth", &unscaled_depth);
	depth_ = c->request_height(scale.length_in(unscaled_depth));
	ifc_object * d;
	object_field(obj, "ExtrudedDirection", &d);
	double dx, dy, dz;
	triple_field(*d, "DirectionRatios", &dx, &dy, &dz);
	dir_ = c->request_direction(dx, dy, dz);
	if (!normal_matches_dir(area_, dir_)) {
		area_.reverse();
	}
	assert(!are_perpendicular(area_.normal(), dir_));
}

void ext::transform(const transformation_3 & t) {
	assert(!are_perpendicular(area_.normal(), dir_));
	area_.transform(t);
	dir_ = dir_.transform(t);
	if (axes_) { 
		axes_ = std::make_tuple(axis1()->transform(t), axis2()->transform(t));
	}
	assert(!are_perpendicular(area_.normal(), dir_));
}

interface_solid ext::to_interface_solid() const {
	interface_solid res;
	res.rep_type = REP_EXT;
	auto & rep = res.rep.as_ext;
	rep.area = area_.to_interface();
	rep.extrusion_depth = CGAL::to_double(depth_);
	rep.ext_dx = CGAL::to_double(dir_.dx());
	rep.ext_dy = CGAL::to_double(dir_.dy());
	rep.ext_dz = CGAL::to_double(dir_.dz());
	return res;
}

std::vector<approximated_curve> ext::approximations() const {
	auto res = area_.approximations();
	size_t count = res.size();
	auto extrude = build_translation(dir_, depth_);
	for (size_t i = 0; i < count; ++i) {
		res.push_back(res[i].transformed(extrude));
	}
	return res;
}

transformation_3 get_globalizer(
	const ifc_object & obj,
	const unit_scaler & scaler,
	number_collection<K> * c) 
{
	if (is_kind_of(obj, "IfcProduct")) {
		ifc_object * rep;
		object_field(obj, "Representation", &rep);
		auto inner = get_globalizer(*rep, scaler, c);
		ifc_object * placement;
		object_field(obj, "ObjectPlacement", &placement);
		return inner * build_transformation(placement, scaler, c);
	}
	else if (is_instance_of(obj, "IfcProductDefinitionShape")) {
		std::vector<ifc_object *> reps;
		collection_field(obj, "Representations", &reps);
		for (auto r = reps.begin(); r != reps.end(); ++r) {
			if (rep_id(**r) == "Body") { 
				return get_globalizer(**r, scaler, c); 
			}
		}
		throw bad_rep_exception("no 'Body' representation identifier");
	}
	else if (
		is_instance_of(obj, "IfcShapeRepresentation") &&
		rep_id(obj) == "Body")
	{
		// This is NOT redundant with the IfcProductDefinitionShape case.
		// IfcShapeRepresentations can live on their own (inside some mapping
		// instances). Do NOT refactor it out!
		std::vector<ifc_object *> rep_items;
		collection_field(obj, "Items", &rep_items);
		return get_globalizer(*rep_items[0], scaler, c);
	}
	else if (is_instance_of(obj, "IfcFacetedBrep")) {
		return transformation_3();
	}
	else if (is_instance_of(obj, "IfcExtrudedAreaSolid")) {
		return transformation_3();
	}
	else if (is_instance_of(obj, "IfcMappedItem")) {
		ifc_object * ms;
		object_field(obj, "MappingSource", &ms);
		ifc_object * mapped_rep;
		object_field(*ms, "MappedRepresentation", &mapped_rep);
		auto base = get_globalizer(*mapped_rep, scaler, c);
		ifc_object * mapping_origin;
		object_field(*ms, "MappingOrigin", &mapping_origin);
		auto from = build_transformation(mapping_origin, scaler, c);
		ifc_object * mapping_target;
		object_field(obj, "MappingTarget", &mapping_target);
		auto to = build_transformation(mapping_target, scaler, c);
		return base * from * to;
	}
	else if (is_instance_of(obj, "IfcBooleanClippingResult")) {
		return transformation_3();
	}
	else if (is_instance_of(obj, "IfcFaceBasedSurfaceModel")) {
		return transformation_3();
	}
	else {
		throw bad_rep_exception("unknown or unsupported geometry definition");
	}
}

boost::optional<std::tuple<direction_3, direction_3>> get_axes(
	const ifc_object & obj,
	number_collection<K> * c)
{
	if (is_instance_of(obj, "IfcPolyline")) {
		std::vector<ifc_object *> pts;
		collection_field(obj, "Points", &pts);
		if (pts.size() != 2) {
			throw bad_rep_exception(
				"axis definition without exactly two points");
		}
		double p1x, p1y, p2x, p2y, dummy;
		triple_field(*pts[0], "Coordinates", &p1x, &p1y, &dummy);
		triple_field(*pts[1], "Coordinates", &p2x, &p2y, &dummy);
		direction_3 a1 = c->request_direction(p2x - p1x, p2y - p1y, 0.0);
		vector_3 a3(0, 0, 1);
		direction_3 a2 = CGAL::cross_product(a3, a1.to_vector()).direction();
		return std::make_tuple(a1, a2);
	}
	return boost::optional<std::tuple<direction_3, direction_3>>();
}

std::unique_ptr<solid> get_local_geometry(
	const ifc_object & obj,
	const unit_scaler & scaler,
	number_collection<K> * c)
{
	if (is_kind_of(obj, "IfcProduct")) {
		ifc_object * rep;
		object_field(obj, "Representation", &rep);
		return get_local_geometry(*rep, scaler, c);
	}
	else if (is_instance_of(obj, "IfcProductDefinitionShape")) {
		std::vector<ifc_object *> reps;
		collection_field(obj, "Representations", &reps);
		std::unique_ptr<solid> geometry;
		boost::optional<std::tuple<direction_3, direction_3>> axes;
		for (auto r = reps.begin(); r != reps.end(); ++r) {
			std::string identifier;
			string_field(**r, "RepresentationIdentifier", &identifier);
			if (identifier == "Body") {
				geometry = get_local_geometry(**r, scaler, c);
			}
			else if (identifier == "Axis") {
				std::vector<ifc_object *> items;
				collection_field(**r, "Items", &items);
				axes = get_axes(*items[0], c);
			}
		}
		if (geometry) {
			geometry->set_axes(axes);
			return geometry;
		}
		throw bad_rep_exception("no 'Body' representation identifier");
	}
	else if (
		is_instance_of(obj, "IfcShapeRepresentation") &&
		rep_id(obj) == "Body")
	{
		// This is NOT redundant with the IfcProductDefinitionShape case.
		// IfcShapeRepresentations can live on their own (inside some mapping
		// instances). Do NOT refactor it out!
		std::vector<ifc_object *> items;
		collection_field(obj, "Items", &items);
		return get_local_geometry(*items[0], scaler, c);
	}
	else if (is_instance_of(obj, "IfcFacetedBrep")) {
		ifc_object * b;
		object_field(obj, "Outer", &b);
		return std::unique_ptr<brep>(new brep(*b, scaler, c));
	}
	else if (is_instance_of(obj, "IfcExtrudedAreaSolid")) {
		auto res = std::unique_ptr<ext>(new ext(obj, scaler, c));
		ifc_object * pos;
		object_field(obj, "Position", &pos);
		res->transform(build_transformation(pos, scaler, c));
		return std::move(res);
	}
	else if (is_instance_of(obj, "IfcMappedItem")) {
		ifc_object * src;
		object_field(obj, "MappingSource", &src);
		ifc_object * rep;
		object_field(*src, "MappedRepresentation", &rep);
		return get_local_geometry(*rep, scaler, c);
	}
	else if (is_instance_of(obj, "IfcBooleanClippingResult")) {
		return wrapped_nef_operations::from_boolean_result(obj, scaler, c);
	}
	else if (is_instance_of(obj, "IfcFaceBasedSurfaceModel")) {
		std::vector<ifc_object *> face_set;
		collection_field(obj, "FbsmFaces", &face_set);
		return std::unique_ptr<brep>(new brep(*face_set[0], scaler, c));
	}
	else {
		throw bad_rep_exception("unknown or unsupported geometry definition");
	}
}

} // namespace internal_geometry