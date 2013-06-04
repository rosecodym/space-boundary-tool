#pragma once

#include "precompiled.h"

#include "equality_context.h"
#include "multiview_solid.h"
#include "oriented_area.h"
#include "report.h"
#include "sbt-core.h"

class element {
private:
	std::string name_;
	multiview_solid geometry_;
	element_type type_;
	element_id_t id_;

	element(const element & info, multiview_solid && new_geometry) 
		: name_(info.name_), 
		geometry_(std::move(new_geometry)),
		type_(info.type_),
		id_(info.id_)
	{ }

public:
	element(const element_info * e, equality_context * c)
		: name_(e->name),
		geometry_(e->geometry, c),
		type_(e->type),
		id_(e->id)
	{ }

	element(element && src)
		: name_(std::move(src.name_)),
		geometry_(std::move(src.geometry_)),
		type_(src.type_),
		id_(src.id_)
	{ }

	element & operator = (element && src) {
		if (&src != this) {
			name_ = std::move(src.name_);
			geometry_ = std::move(src.geometry_);
			type_ = src.type_;
			id_ = src.id_;
		}
		return *this;
	}

	const std::string & name() const { return name_; }
	element_type type() const { return type_; }
	element_id_t material() const { return id_; }
	const multiview_solid & geometry() const { return geometry_; }
	bool is_fenestration() const { return type_ == DOOR || type_ == WINDOW; }
	bbox_3 bounding_box() const { return geometry_.bounding_box(); }

	std::vector<oriented_area> faces(equality_context * c) const { 
		return geometry_.oriented_faces(c); 
	}

	void subtract_geometry_of(const element & other, equality_context * c) {
		geometry_.subtract(other.geometry_, c); 
	}

	static bool share_plane_opposite(
		const element & a, 
		const element & b,
		equality_context * c)
	{
		return multiview_solid::share_plane_opposite(
			a.geometry(),
			b.geometry(),
			c);
	}

	template <typename OutputIterator>
	static void explode_to_single_volumes(element && src, equality_context * c, OutputIterator oi) {
		reporting::report_progress(boost::format(
			"Checking element %s for multiple volumes: ") % src.name());
		if (src.geometry_.is_single_volume()) {
			reporting::report_progress("element is a single volume.\n");
			*oi++ = std::move(src);
		}
		else {
			reporting::report_progress("element is multiple volumes. Converting to single volumes");
			auto single_volumes = src.geometry_.as_single_volumes(c);
			for (auto v = single_volumes.begin(); v != single_volumes.end(); ++v) {
				*oi++ = element(src, std::move(*v));
				reporting::report_progress(".");
			}
			reporting::report_progress("done.\n");
		}
	}
};
