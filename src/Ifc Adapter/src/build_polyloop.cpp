#include "precompiled.h"

#include "../../Edm Wrapper/edm_wrapper_native_interface.h"

#include "build_polyloop.h"

#include "approximated_curve.h"
#include "geometry_common.h"
#include "ifc-to-cgal.h"
#include "internal_geometry.h"
#include "number_collection.h"
#include "unit_scaler.h"

namespace internal_geometry {

using namespace ifc_interface;

namespace {

typedef std::vector<point_3> loop;
typedef std::vector<approximated_curve> approximations;
typedef std::tuple<loop, approximations> polyloop_result;

typedef std::function<polyloop_result(
	const ifc_object &, 
	const unit_scaler &, 
	number_collection<K> *)> polyloop_builder;

class handler {
public:
	handler(
		const std::string & ifc_type,
		bool exact,
		const polyloop_builder & action)
		: ifc_type_(ifc_type),
		  exact_(exact),
		  action_(action)
	{ }
	boost::optional<polyloop_result> handle(
		const ifc_object & obj, 
		const unit_scaler & s,
		number_collection<K> * c) const
	{
		if ((exact_ && is_instance_of(obj, ifc_type_.c_str())) ||
			is_kind_of(obj, ifc_type_.c_str()))
		{
			return action_(obj, s, c);
		}
		else { return boost::optional<polyloop_result>(); }
	}
private:
	std::string ifc_type_;
	bool exact_;
	polyloop_builder action_;
};

const approximations NO_APPROXES;

polyloop_result from_polyline(
	const ifc_object & obj, 
	const unit_scaler & s, 
	number_collection<K> * c)
{
	loop res;
	auto pts = collection_field(obj, "Points");
	for (auto p = pts.begin(); p != pts.end(); ++p) {
		res.push_back(build_point(**p, s, c));
	}
	if (res.front() == res.back()) { res.pop_back(); }
	return std::make_tuple(res, NO_APPROXES);
}

polyloop_result from_composite_curve_segment(
	const ifc_object & obj,
	const unit_scaler & s,
	number_collection<K> * c)
{
	return build_polyloop(*object_field(obj, "ParentCurve"), s, c);
}

polyloop_result from_composite_curve(
	const ifc_object & obj,
	const unit_scaler & s,
	number_collection<K> * c)
{
	auto components = collection_field(obj, "Segments");
	if (components.size() != 1) {
		throw bad_rep_exception(
			"composite curves without exactly one segment are unsupported");
	}
	return build_polyloop(*components[0], s, c);
}

polyloop_result from_polyloop(
	const ifc_object & obj,
	const unit_scaler & s,
	number_collection<K> * c)
{
	loop res;
	auto pts = collection_field(obj, "Polygon");
	for (auto p = pts.begin(); p != pts.end(); ++p) {
		res.push_back(build_point(**p, s, c));
	}
	return std::make_tuple(res, NO_APPROXES);
}

polyloop_result from_curve_bounded_plane(
	const ifc_object & obj,
	const unit_scaler & s,
	number_collection<K> * c)
{
	auto bound = object_field(obj, "OuterBoundary");
	loop res;
	std::vector<approximated_curve> approxes;
	std::tie(res, approxes) = build_polyloop(*bound, s, c);
	auto basis = object_field(obj, "BasisSurface");
	auto t = build_transformation(basis, s, c);
	boost::transform(res, res.begin(), t);
	for (auto p = approxes.begin(); p != approxes.end(); ++p) {
		*p = p->transformed(t);
	}
	return std::make_tuple(res, NO_APPROXES);
}

polyloop_result from_arbitrary_closed_profile_def(
	const ifc_object & obj,
	const unit_scaler & s,
	number_collection<K> * c)
{
	return build_polyloop(*object_field(obj, "OuterCurve"), s, c);
}

polyloop_result from_rectangle_profile_def(
	const ifc_object & obj,
	const unit_scaler & scale,
	number_collection<K> * c)
{
	double xdim = real_field(obj, "XDim");
	double ydim = real_field(obj, "YDim");
	point_2 req;
	loop res;
	auto s = [&scale](double d) { return scale.length_in(d); };
	req = c->request_point(s(-xdim) / 2, s(-ydim) / 2);
	res.push_back(point_3(req.x(), req.y(), 0));
	req = c->request_point(s(xdim) / 2, s(-ydim) / 2);
	res.push_back(point_3(req.x(), req.y(), 0));
	req = c->request_point(s(xdim) / 2, s(ydim) / 2);
	res.push_back(point_3(req.x(), req.y(), 0));
	req = c->request_point(s(-xdim) / 2, s(ydim) / 2);
	res.push_back(point_3(req.x(), req.y(), 0));
	auto t = build_transformation(object_field(obj, "Position"), scale, c);
	boost::transform(res, res.begin(), t);
	assert(boost::find_if(res, [](const point_3 & p) {
		return !CGAL::is_zero(p.z());
	}) == res.end());
	return std::make_tuple(res, NO_APPROXES);
}

polyloop_result from_face_bound(
	const ifc_object & obj,
	const unit_scaler & s,
	number_collection<K> * c)
{
	auto res = build_polyloop(*object_field(obj, "Bound"), s, c);
	if (!boolean_field(obj, "Orientation")) { 
		boost::reverse(std::get<0>(res)); 
	}
	return res;
}

polyloop_result from_circle_profile_def(
	const ifc_object & /*obj*/,
	const unit_scaler & /*s*/,
	number_collection<K> * /*c*/)
{
	throw bad_rep_exception("curved geometry definitions are not supported.");
}

} // namespace

polyloop_result build_polyloop(
	const ifc_object & obj, 
	const unit_scaler & s, 
	number_collection<K> * c) 
{
	std::array<handler, 9> handlers = {
		handler("IfcPolyline", true, &from_polyline),
		handler("IfcCompositeCurveSegment", true, &from_composite_curve_segment),
		handler("IfcCompositeCurve", true, &from_composite_curve),
		handler("IfcPolyLoop", true, &from_polyloop),
		handler("IfcCurveBoundedPlane", true, &from_curve_bounded_plane),
		handler("IfcArbitraryClosedProfileDef", true, &from_arbitrary_closed_profile_def),
		handler("IfcRectangleProfileDef", true, &from_rectangle_profile_def),
		handler("IfcFaceBound", false, &from_face_bound),
		handler("IfcCircleProfileDef", false, &from_circle_profile_def)
	};

	for (auto p = handlers.begin(); p != handlers.end(); ++p) {
		auto res = p->handle(obj, s, c);
		if (res) { return *res; }
	}

	throw bad_rep_exception("unsupported representation for a polyloop");
}

} // namespace internal_geometry