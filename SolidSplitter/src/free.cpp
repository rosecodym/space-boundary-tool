#include "precompiled.h"

#include "../../Core/src/sbt-core.h"

#include "free.h"

namespace {

void free_interior(const polyloop & p) {
	free(p.vertices);
}

void free_interior(const face & f) {
	free_interior(f.outer_boundary);
	if (f.void_count > 0) {
		for (size_t i = 0; i < f.void_count; ++i) {
			free_interior(f.voids[i]);
		}
		free(f.voids);
	}
}

void free_interior(const brep & b) {
	for (size_t i = 0; i < b.face_count; ++i) {
		free_interior(b.faces[i]);
	}
	free(b.faces);
}

void free_interior(const extruded_area_solid & e) {
	free_interior(e.area);
}

} // namespace

void free_interior(const solid & s) {
	if (s.rep_type == REP_BREP) {
		free_interior(s.rep.as_brep);
	}
	else {
		free_interior(s.rep.as_ext);
	}
}