#pragma once

#include "precompiled.h"

#include "solid_geometry_common.h"

class equality_context;

namespace solid_geometry {

namespace impl {

nef_polyhedron_3				extrusion_to_nef(const extrusion_information & ext, equality_context * c);
std::tuple<size_t, face_status>	find_root(const std::vector<simple_face> & all_faces, const std::vector<int> & group_memberships, int group);

} // namespace impl

} // namespace solid_geometry