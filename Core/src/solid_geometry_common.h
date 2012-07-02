#pragma once

#include "precompiled.h"

#include "oriented_area.h"
#include "simple_face.h"

namespace solid_geometry {

namespace impl {

typedef std::vector<std::vector<simple_face>> simple_face_groups;
typedef std::vector<std::vector<oriented_area>> oriented_area_groups;
typedef std::tuple<simple_face, vector_3> extrusion_information;

} // namespace impl

} // namespace solid_geometry