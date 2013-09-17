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

loop approximate_circle(
	const ifc_object & circle_like,
	const unit_scaler & scaler,
	number_collection<K> * c)
{
	// Create a square with area equal to the provided circle. The square will
	// lie in a plane parallel to the xy plane.
	double r;
	real_field(circle_like, "Radius", &r);
	double diff = sqrt(3.14159) * r / 2;
	ifc_object * pos;
	object_field(circle_like, "Position", &pos);
	ifc_object * center;
	object_field(*pos, "Location", &center);
	double x, y, z;
	triple_field(*center, "Coordinates", &x, &y, &z);
	assert(z == 0.0);
	auto s = [&scaler] (double d) { return scaler.length_in(d); };
	loop res;
	res.push_back(c->request_point(s(x + diff), s(y - diff), z));
	res.push_back(c->request_point(s(x + diff), s(y + diff), z));
	res.push_back(c->request_point(s(x - diff), s(y + diff), z));
	res.push_back(c->request_point(s(x - diff), s(y - diff), z));
	return res;
}

boost::optional<loop> is_split_circle(
	const std::vector<ifc_object *> & parts,
	const unit_scaler & s,
	number_collection<K> * c)
{
	// Sometimes circles are broken up into pieces for no reason.
	const ifc_object * circle = nullptr;
	boost::optional<direction_3> x;
	boost::optional<direction_3> y;
	boost::optional<point_3> center;
	for (auto p = parts.begin(); p != parts.end(); ++p) {
		ifc_object * parent;
		object_field(**p, "ParentCurve", &parent);
		if (is_instance_of(*parent, "IfcTrimmedCurve")) {
			ifc_object * basis;
			object_field(*parent, "BasisCurve", &basis);
			if (is_instance_of(*basis, "IfcCircle")) {
				ifc_object * this_geom;
				object_field(*basis, "Position", &this_geom);
				std::vector<ifc_object *> this_axes;
				collection_field(*this_geom, "P", &this_axes);
				auto this_x = build_direction(*this_axes[0], c);
				if (circle && this_x != x) { return boost::optional<loop>(); }
				else { x = this_x; }
				auto this_y = build_direction(*this_axes[1], c);
				if (circle && this_y != y) { return boost::optional<loop>(); }
				else { y = this_y; }
				ifc_object * this_loc;
				object_field(*this_geom, "Location", &this_loc);
				auto this_center = build_point(*this_loc, s, c);
				if (circle && this_center != center) { 
					return boost::optional<loop>();
				}
				else { center = this_center; }
				circle = basis;
			}
			else { return boost::optional<loop>(); }
		}
		else { return boost::optional<loop>(); }
	}
	return approximate_circle(*circle, s, c);
}

loop polyline_points(
	const ifc_object & obj, 
	const unit_scaler & s, 
	number_collection<K> * c)
{
	loop res;
	std::vector<ifc_object *> pts;
	collection_field(obj, "Points", &pts);
	for (auto p = pts.begin(); p != pts.end(); ++p) {
		res.push_back(build_point(**p, s, c));
	}
	if (res.front() == res.back()) { res.pop_back(); }
	return res;
}

std::tuple<point_3, point_3, boost::optional<approximated_curve>>
handle_trimmed_curve(
	const ifc_object & obj,
	const unit_scaler & s,
	number_collection<K> * c)
{
	ifc_object * basis;
	object_field(obj, "BasisCurve", &basis);
	std::vector<ifc_object *> trim1, trim2;
	collection_field(obj, "Trim1", &trim1);
	collection_field(obj, "Trim2", &trim2);
	boost::optional<point_3> from, to;
	boost::optional<approximated_curve> a;
	if (is_instance_of(*basis, "IfcEllipse")) {
		throw bad_rep_exception("ellipses are not yet supported");
	}
	else if (is_instance_of(*basis, "IfcLine")) {
		for (auto p = trim1.begin(); p != trim1.end(); ++p) {
			if (is_instance_of(**p, "IfcCartesianPoint")) {
				from = build_point(**p, s, c);
				break;
			}
		}
		for (auto p = trim2.begin(); p != trim2.end(); ++p) {
			if (is_instance_of(**p, "IfcCartesianPoint")) {
				to = build_point(**p, s, c);
				break;
			}
		}
		if (!from || !to) {
			throw bad_rep_exception("lines trimmed by parameters instead of points are not yet supported");
		}
		bool sense;
		boolean_field(obj, "SenseAgreement", &sense);
		boost::optional<approximated_curve> no_approx;
		if (sense) { return std::make_tuple(*from, *to, no_approx); }
		else { return std::make_tuple(*to, *from, no_approx); }
	}
	else { // Per the 2x3 standard, it has to be an IfcCircle
		boost::optional<double> p1, p2;
		for (auto p = trim1.begin(); p != trim1.end(); ++p) {
			if (is_instance_of(**p, "IfcCartesianPoint")) {
				from = build_point(**p, s, c);
			}
			else { 
				double param;
				as_real(**p, &param);
				p1 = param * 3.14159 / 180; 
			}
		}
		for (auto p = trim2.begin(); p != trim2.end(); ++p) {
			if (is_instance_of(**p, "IfcCartesianPoint")) {
				to = build_point(**p, s, c);
			}
			else { 
				double param;
				as_real(**p, &param);
				p2 = param * 3.14159 / 180; 
			}
		}
		double radius;
		real_field(*basis, "Radius", &radius);
		radius = s.length_in(radius);
		ifc_object * position;
		object_field(*basis, "Position", &position);
		auto t = build_transformation(position, s, c);
		auto rebuilt = t(point_3(radius * cos(*p1), radius * sin(*p1), 0));
		if (!from) { from = rebuilt; }
		else if (p1) { 
#ifndef NDEBUG
			double rx = CGAL::to_double(rebuilt.x());
			double ry = CGAL::to_double(rebuilt.y());
			double ox = CGAL::to_double(from->x());
			double oy = CGAL::to_double(from->y());
			double dist = sqrt((rx - ox) * (rx - ox) + (ry - oy) * (ry - oy));
			assert(c->is_zero(dist));
#endif
		}
		rebuilt = t(point_3(radius * cos(*p2), radius * sin(*p2), 0));
		if (!to) { to = rebuilt; }
		else if (p2) {
#ifndef NDEBUG
			double rx = CGAL::to_double(rebuilt.x());
			double ry = CGAL::to_double(rebuilt.y());
			double ox = CGAL::to_double(to->x());
			double oy = CGAL::to_double(to->y());
			double dist = sqrt((rx - ox) * (rx - ox) + (ry - oy) * (ry - oy));
			assert(c->is_zero(dist));
#endif
		}
		// While calculating the true area and length is easy using the given
		// parameters, they aren't always present, so we might as well just
		// reconstruct them in all cases.
		auto inv_t = t.inverse();
		auto c13d = inv_t(*from);
		auto c23d = inv_t(*to);
		assert(c->is_zero(CGAL::to_double(c13d.z())));
		assert(c->is_zero(CGAL::to_double(c23d.z())));
		double c1x = CGAL::to_double(c13d.x());
		double c1y = CGAL::to_double(c13d.y());
		double c2x = CGAL::to_double(c23d.x());
		double c2y = CGAL::to_double(c23d.y());
		double angle1 = atan2(c1y, c1x);
		double angle2 = atan2(c2y, c2x);
		assert(!p1 || angles_equal(*p1, angle1, *c));
		assert(!p2 || angles_equal(*p2, angle2, *c));
		bool sense;
		boolean_field(obj, "SenseAgreement", &sense);
		double raw = sense ? angle2 - angle1 : angle1 - angle2;
		double diff = fmod(fmod(raw, 2 * 3.14159) + 2 * 3.14159, 2 * 3.14159);
		assert(diff >= 0.0);
		assert(diff < 2 * 3.14159);
		double true_length = radius * diff;
		double dist = sqrt(CGAL::to_double((*to - *from).squared_length()));
		// Formula from http://en.wikipedia.org/wiki/Circle_segment
		double area = radius * radius / 2.0 * (diff - sin(diff));
		direction_3 n(0.0, 0.0, 1.0);
		if (sense) {
			a = approximated_curve(*from, *to, n, -area, true_length / dist);
			return std::make_tuple(*from, *to, a);
		}
		else {
			a = approximated_curve(*to, *from, n, -area, true_length / dist);
			return std::make_tuple(*to, *from, a);
		}
	}
}

polyloop_result from_polyline(
	const ifc_object & obj, 
	const unit_scaler & s, 
	number_collection<K> * c)
{
	return std::make_tuple(polyline_points(obj, s, c), NO_APPROXES);
}

polyloop_result from_composite_curve(
	const ifc_object & obj,
	const unit_scaler & s,
	number_collection<K> * c)
{
	std::vector<ifc_object *> components;
	collection_field(obj, "Segments", &components);
	auto as_circle = is_split_circle(components, s, c);
	if (as_circle) { return std::make_tuple(*as_circle, NO_APPROXES); }
	loop res;
	approximations all_approxes;
	for (auto p = components.begin(); p != components.end(); ++p) {
		ifc_object * parent;
		object_field(**p, "ParentCurve", &parent);
		loop pts;
		approximations approxes;
		if (is_instance_of(*parent, "IfcCompositeCurve")) {
			std::tie(pts, approxes) = from_composite_curve(*parent, s, c);
		}
		else if (is_instance_of(*parent, "IfcPolyline")) {
			pts = polyline_points(*parent, s, c);
		}
		else if (is_instance_of(*parent, "IfcTrimmedCurve")) {
			point_3 from;
			point_3 to;
			boost::optional<approximated_curve> a;
			std::tie(from, to, a) = handle_trimmed_curve(*parent, s, c);
			auto append_unique = [&pts](const point_3 & p) {
				if (pts.empty() || pts.back() != p) { pts.push_back(p); }
			};
			bool same_sense;
			boolean_field(**p, "SameSense", &same_sense);
			if (same_sense) {
				append_unique(from);
				append_unique(to);
			}
			else {
				append_unique(to);
				append_unique(from);
			}
			if (a) { approxes.push_back(*a); }
		}
		else {
			throw bad_rep_exception("unsupported composite curve segment");
		}
		boost::copy(pts, std::back_inserter(res));
		boost::copy(approxes, std::back_inserter(all_approxes));
	}
	return std::make_tuple(res, all_approxes);
}

polyloop_result from_polyloop(
	const ifc_object & obj,
	const unit_scaler & s,
	number_collection<K> * c)
{
	loop res;
	std::vector<ifc_object *> pts;
	collection_field(obj, "Polygon", &pts);
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
	ifc_object * bound;
	object_field(obj, "OuterBoundary", &bound);
	loop res;
	std::vector<approximated_curve> approxes;
	std::tie(res, approxes) = build_polyloop(*bound, s, c);
	ifc_object * basis;
	object_field(obj, "BasisSurface", &basis);
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
	ifc_object * outer;
	object_field(obj, "OuterCurve", &outer);
	return build_polyloop(*outer, s, c);
}

polyloop_result from_rectangle_profile_def(
	const ifc_object & obj,
	const unit_scaler & scale,
	number_collection<K> * c)
{
	double xdim, ydim;
	real_field(obj, "XDim", &xdim);
	real_field(obj, "YDim", &ydim);
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
	ifc_object * pos;
	object_field(obj, "Position", &pos);
	auto t = build_transformation(pos, scale, c);
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
	ifc_object * bound;
	object_field(obj, "Bound", &bound);
	auto res = build_polyloop(*bound, s, c);
	bool orientation;
	boolean_field(obj, "Orientation", &orientation);
	if (!orientation) { boost::reverse(std::get<0>(res)); }
	return res;
}

polyloop_result from_circle_profile_def(
	const ifc_object & obj,
	const unit_scaler & s,
	number_collection<K> * c)
{
	return std::make_tuple(approximate_circle(obj, s, c), NO_APPROXES);
}

} // namespace

polyloop_result build_polyloop(
	const ifc_object & obj, 
	const unit_scaler & s, 
	number_collection<K> * c) 
{
	std::array<handler, 8> handlers = {
		handler("IfcPolyline", true, &from_polyline),
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