#pragma once

#include "precompiled.h"

#include "solid_geometry_common.h"

class equality_context;

namespace solid_geometry {

namespace impl {

nef_polyhedron_3 extrusion_to_nef(const extrusion_information & ext, equality_context * c);
nef_polyhedron_3 simple_faces_to_nef(std::vector<simple_face> && all_faces);

} // namespace impl

} // namespace solid_geometry