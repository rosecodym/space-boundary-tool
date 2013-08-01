#pragma once

#include "precompiled.h"

#include "number_collection.h"
#include "sbt-ifcadapter.h"

class unit_scaler;

namespace ifc_interface {

class ifc_object;

} // namespace ifc_interface

namespace internal_geometry {

typedef std::function<double(double)> length_scaler;

typedef ::face interface_face;
typedef ::solid interface_solid;

class face {
private:
	std::vector<point_3> outer_;
	std::vector<std::vector<point_3>> voids_;
public:
	face(
		const ifc_interface::ifc_object & obj,
		const length_scaler & scale_length,
		number_collection<K> * c);

	explicit face(const std::vector<point_3> & points) : outer_(points) { }

	const std::vector<point_3> & outer_boundary() const { return outer_; }
	const std::vector<std::vector<point_3>> & voids() const { return voids_ ; }
	direction_3 normal() const;
	interface_face to_interface() const;
	void reverse();
	void transform(const transformation_3 & t);
};

class solid {
protected:
	boost::optional<std::tuple<direction_3, direction_3>> axes_;
public:
	boost::optional<direction_3> axis1() const {
		if (axes_) { return std::get<0>(*axes_); }
		else { return boost::optional<direction_3>(); }
	}
	boost::optional<direction_3> axis2() const {
		if (axes_) { return std::get<1>(*axes_); }
		else { return boost::optional<direction_3>(); }
	}
	boost::optional<direction_3> axis3() const {
		if (axes_) {
			direction_3 a1 = std::get<0>(*axes_);
			direction_3 a2 = std::get<1>(*axes_);
			auto cross = CGAL::cross_product(a1.to_vector(), a2.to_vector());
			return cross.direction();
		}
		else { return boost::optional<direction_3>(); }
	}
	void set_axes(
		const boost::optional<std::tuple<direction_3, direction_3>> & a)
	{
		axes_ = a;
	}

	virtual void transform(const transformation_3 & t) = 0;
	virtual interface_solid to_interface_solid() const = 0;
};

class brep : public solid {
private:
	std::vector<face> faces;
public:
	brep(
		const ifc_interface::ifc_object & inst, 
		const length_scaler & scale_length,
		number_collection<K> * c);

	explicit brep(const std::vector<face> & faces) : faces(faces) { }

	virtual void transform(const transformation_3 & t);
	virtual interface_solid to_interface_solid() const;
};

class ext : public solid {
private:
	face area_;
	direction_3 dir_;
	NT depth_;
public:
	ext(
		const ifc_interface::ifc_object & inst, 
		const length_scaler & scale_length,
		number_collection<K> * c);
	const face & base() const { return area_; }
	const direction_3 & dir() const { return dir_; }
	const NT & depth() const { return depth_; }
	virtual void transform(const transformation_3 & t);
	virtual interface_solid to_interface_solid() const;
};

class bad_rep_exception : public std::exception {
private:
	std::string msg;
public:
	bad_rep_exception(const std::string & msg) : msg(msg) { }
	virtual const char * what() const throw() { return msg.c_str(); }
};

transformation_3 get_globalizer(
	const ifc_interface::ifc_object & obj, 
	const unit_scaler & scaler,
	number_collection<K> * c);

std::unique_ptr<solid> get_local_geometry(
	const ifc_interface::ifc_object & obj, 
	const unit_scaler & scaler,
	number_collection<K> * c);

} // namespace internal_geometry