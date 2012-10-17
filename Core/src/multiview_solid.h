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

	typedef boost::variant<oriented_area_groups, extrusion_information, nef_polyhedron_3> geometry_representation;

	mutable geometry_representation geometry;

	bool is_nef_representable() const;

	void convert_to_nef(std::function<equality_context *(void)>) const;

	multiview_solid(const std::vector<oriented_area> & oriented_area_volume) : geometry(oriented_area_groups()) { boost::get<oriented_area_groups>(geometry).push_back(oriented_area_volume); }
	multiview_solid(const extrusion_information & ext) : geometry(ext) { }
	multiview_solid(const nef_polyhedron_3 & nef, nef_volume_handle v, equality_context * c);

public:
	multiview_solid(const solid & s, equality_context * c);

	multiview_solid(multiview_solid && src) { *this = std::move(src); }
	multiview_solid & operator = (multiview_solid && src);

	bbox_3 bounding_box() const;
	bool is_single_volume() const;
	std::vector<multiview_solid> as_single_volumes(equality_context * c) const;
	std::vector<oriented_area> oriented_faces(equality_context * c) const;

	void subtract(const multiview_solid & other, equality_context * c);
};

} // namespace solid_geometry

typedef solid_geometry::multiview_solid multiview_solid;