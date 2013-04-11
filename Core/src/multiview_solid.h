#pragma once

#include "precompiled.h"

#include "equality_context.h"
#include "oriented_area.h"
#include "sbt-core.h"
#include "simple_face.h"
#include "solid_geometry_common.h"

namespace solid_geometry {

class multiview_solid {
private:
	typedef impl::oriented_area_groups oriented_area_groups;
	typedef impl::extrusion_information extrusion_information;

	mutable boost::optional<oriented_area_groups> as_face_groups_;
	mutable boost::optional<extrusion_information> as_extrusion_info_;
	mutable boost::optional<nef_polyhedron_3> as_nef_;

	bool is_nef_representable() const;
	void create_nef_rep(std::function<equality_context *(void)> lazy_c) const;

	multiview_solid(const std::vector<oriented_area> & oriented_area_volume) 
		: as_face_groups_(oriented_area_groups(1, oriented_area_volume)) { }
	multiview_solid(const extrusion_information & ext) 
		: as_extrusion_info_(ext) { }
	multiview_solid(
		const nef_polyhedron_3 & nef, 
		nef_volume_handle v, 
		equality_context * c);

public:
	multiview_solid(const solid & s, equality_context * c);

	multiview_solid(multiview_solid && src) 
		: as_face_groups_(std::move(src.as_face_groups_)),
		  as_extrusion_info_(std::move(src.as_extrusion_info_)),
		  as_nef_(std::move(src.as_nef_)) { }
	multiview_solid & operator = (multiview_solid && src) { 
		as_face_groups_ = std::move(src.as_face_groups_);
		as_extrusion_info_ = std::move(src.as_extrusion_info_);
		as_nef_ = std::move(src.as_nef_);
		return *this;
	}

	bbox_3 bounding_box() const;
	bool is_single_volume() const;
	std::vector<multiview_solid> as_single_volumes(equality_context * c) const;
	std::vector<oriented_area> oriented_faces(equality_context * c) const;

	void subtract(const multiview_solid & other, equality_context * c);

	static bool share_plane_opposite(
		const multiview_solid & a,
		const multiview_solid & b,
		equality_context * c);
};

} // namespace solid_geometry

typedef solid_geometry::multiview_solid multiview_solid;