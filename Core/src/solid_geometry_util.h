#pragma once

#include "precompiled.h"

#include "solid_geometry_common.h"

class equality_context;

namespace solid_geometry {

namespace impl {

nef_polyhedron_3			extrusion_to_nef(const extrusion_information & ext, equality_context * c);
std::vector<simple_face>	faces_from_brep(const brep & b, equality_context * c);
nef_polyhedron_3			simple_faces_to_nef(std::vector<simple_face> && all_faces);
nef_polyhedron_3			volume_group_to_nef(const std::vector<simple_face> & group);

} // namespace impl

} // namespace solid_geometry