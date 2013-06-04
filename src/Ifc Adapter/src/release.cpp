#include "precompiled.h"

#include "sbt-ifcadapter.h"

#include "release.h"

namespace {

void release_components(polyloop loop) {
	free(loop.vertices);
}

void release_components(face f) {
	release_components(f.outer_boundary);
	for (size_t i = 0; i < f.void_count; ++i) {
		release_components(f.voids[i]);
	}
	free(f.voids);
}

void release_components(brep b) {
	for (size_t i = 0; i < b.face_count; ++i) {
		release_components(b.faces[i]);
	}
	free(b.faces);
}

void release_components(extruded_area_solid ext) {
	release_components(ext.area);
}

void release_components(solid s) {
	if (s.rep_type == REP_BREP) { release_components(s.rep.as_brep); }
	else if (s.rep_type == REP_EXT) { release_components(s.rep.as_ext); }
}

} // namespace

void release(element_info * info) {
	release_components(info->geometry);
	free(info);
}

void release(space_info * info) {
	release_components(info->geometry);
	free(info);
}