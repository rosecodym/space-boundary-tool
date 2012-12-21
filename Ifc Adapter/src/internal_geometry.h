#include "precompiled.h"

#include "cgal-typedefs.h"
#include "exact_surrogates.h"
#include "number_collection.h"
#include "sbt-ifcadapter.h"

class unit_scaler;

namespace internal_geometry {

typedef std::function<double(double)> length_scaler;

typedef ::face interface_face;
typedef ::solid interface_solid;

class face {
private:
	std::vector<point_3> outer;
	std::vector<std::vector<point_3>> voids;
public:
	face(
		const cppw::Select & sel,
		const length_scaler & scale_length,
		number_collection<K> * c);
	// Legacy facade
	face(const exact_face & f);
	direction_3 normal() const;
	interface_face to_interface() const;
	void reverse();
	void transform(const transformation_3 & t);
};

class solid {
protected:
	boost::optional<direction_3> axis_;
public:
	const boost::optional<direction_3> & get_axis() const { return axis_; }
	void set_axis(const boost::optional<direction_3> & a) { axis_ = a; }

	virtual void transform(const transformation_3 & t) = 0;
	virtual interface_solid to_interface_solid() const = 0;
	
	static std::unique_ptr<solid> legacy_facade_build(const exact_solid & s);
};

class brep : public solid {
private:
	std::vector<face> faces;
public:
	brep(
		const cppw::Instance & inst, 
		const length_scaler & scale_length,
		number_collection<K> * c);
	// Legacy facade
	brep(const exact_brep & b);
	virtual void transform(const transformation_3 & t);
	virtual interface_solid to_interface_solid() const;
};

class ext : public solid {
private:
	face area;
	direction_3 dir;
	NT depth;
public:
	ext(
		const cppw::Instance & inst, 
		const length_scaler & scale_length,
		number_collection<K> * c);
	// Legacy facade
	ext(const exact_extruded_area_solid & e);
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
	const cppw::Select & sel, 
	const unit_scaler & scaler,
	number_collection<K> * c);
transformation_3 get_globalizer(
	const cppw::Instance & inst, 
	const unit_scaler & scaler,
	number_collection<K> * c);

std::unique_ptr<solid> get_local_geometry(
	const cppw::Select & sel, 
	const unit_scaler & scaler,
	number_collection<K> * c);
std::unique_ptr<solid> get_local_geometry(
	const cppw::Instance & inst, 
	const unit_scaler & scaler,
	number_collection<K> * c);

} // namespace internal_geometry