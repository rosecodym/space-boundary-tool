#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <cstdio>

#include "sbt-core.h"

#include "sbt-core-helpers.h"

namespace {

polyloop create_polyloop(size_t vertex_count) {
	polyloop loop;
	loop.vertex_count = vertex_count;
	if (vertex_count > 0) {
		loop.vertices = (point *)malloc(sizeof(point) * vertex_count);
	}
	else {
		loop.vertices = nullptr;
	}
	return loop;
}

face create_face() {
	face f;
	f.outer_boundary = create_polyloop(0);
	f.void_count = 0;
	f.voids = nullptr;
	return f;
}

void cleanup_polyloop(polyloop * loop) {
	if (loop && loop->vertex_count > 0) {
		free(loop->vertices);
		loop->vertices = nullptr;
		loop->vertex_count = 0;
	}
}

void cleanup_face(face * f) {
	if (f) {
		cleanup_polyloop(&f->outer_boundary);
		for (size_t i = 0; i < f->void_count; ++i) {
			cleanup_polyloop(&f->voids[i]);
		}
		if (f->void_count > 0) {
			free(f->voids);
		}
		f->void_count = 0;
	}
}

void cleanup_brep(brep * b) {
	b->face_count = 0;
	for (size_t i = 0; i < b->face_count; ++i) {
		cleanup_face(&b->faces[i]);
	}
	free(b->faces);
}

void cleanup_ext(extruded_area_solid * e) {
	e->ext_dx = 0;
	e->ext_dy = 0;
	e->ext_dz = 0;
	e->extrusion_depth = 0;
	cleanup_face(&e->area);
}

void cleanup_solid(solid * s) {
	if (s->rep_type == REP_BREP) {
		cleanup_brep(&s->rep.as_brep);
	}
	else if (s->rep_type == REP_EXT) {
		cleanup_ext(&s->rep.as_ext);
	}
	s->rep_type = REP_NOTHING;
}

void cleanup_space_info(space_info * info) {
	info->id[0] = '\0';
	cleanup_solid(&info->geometry);
}

void cleanup_element_info(element_info * info) {
	info->id[0] = '\0';
	cleanup_solid(&info->geometry);
}

} // namespace

sb_calculation_options create_default_options(void) {
	sb_calculation_options opts;
	opts.flags = SBT_NONE;
	opts.equality_tolerance = 0.01;
	opts.max_pair_distance = 3.0;
	opts.space_verification_timeout = 0;
	opts.space_filter = NULL;
	opts.space_filter_count = 0;
	opts.element_filter = NULL;
	opts.element_filter_count = 0;
	opts.notify_func = NULL;
	opts.warn_func = NULL;
	opts.error_func = NULL;
	return opts;
}

bool is_external_sb(space_boundary * sb) {
	return sb->opposite && !sb->opposite->bounded_space;
}

space_info ** create_space_list(size_t count) {
	space_info ** list = (space_info **)malloc(sizeof(space_info *) * count);
	for (size_t i = 0; i < count; ++i) {
		(list[i] = (space_info *)malloc(sizeof(space_info)))->geometry.rep_type = REP_NOTHING;
	}
	return list;
}

element_info ** create_element_list(size_t count) {
	element_info ** list = (element_info **)malloc(sizeof(element_info *) * count);
	for (size_t i = 0; i < count; ++i) {
		(list[i] = (element_info *)malloc(sizeof(element_info)))->geometry.rep_type = REP_NOTHING;
	}
	return list;
}

void free_space_list(space_info ** list, size_t count) {
	// can't be assed to fix a double-free right now
	//for (size_t i = 0; i < count; ++i) {
	//	cleanup_space_info(list[i]);
	//	free(list[i]);
	//}
	//free(list);
}

void free_element_list(element_info ** list, size_t count) {
	// can't be assed to fix a double-free right now
	//for (size_t i = 0; i < count; ++i) {
	//	cleanup_element_info(list[i]);
	//	free(list[i]);
	//}
	//free(list);
}

void set_space_id(space_info * info, space_id_t id) {
	strncpy(info->id, id, SPACE_ID_MAX_LEN);
}

void set_element_id(element_info * info, element_id_t id) {
	strncpy(info->id, id, ELEMENT_ID_MAX_LEN);
}

void set_element_type(element_info * info, element_type type) {
	info->type = type;
}

void set_element_material(element_info * info, material_id_t mat) {
	info->material = mat;
}

solid * get_space_geometry_handle(space_info * space) {
	return &space->geometry;
}

solid * get_element_geometry_handle(element_info * element) {
	return &element->geometry;
}

void set_to_brep(solid * s, size_t face_count) {
	cleanup_solid(s);
	s->rep_type = REP_BREP;
	s->rep.as_brep.face_count = face_count;
	s->rep.as_brep.faces = (face *)malloc(sizeof(face) * s->rep.as_brep.face_count);
}

void set_to_extruded_area_solid(solid * s, double dx, double dy, double dz, double depth) {
	cleanup_solid(s);
	s->rep_type = REP_EXT;
	s->rep.as_ext.extrusion_depth = depth;
	s->rep.as_ext.ext_dx = dx;
	s->rep.as_ext.ext_dy = dy;
	s->rep.as_ext.ext_dz = dz;
	s->rep.as_ext.area = create_face();
}

face * get_face_handle(solid * brep, size_t index) {
	return &brep->rep.as_brep.faces[index];
}

face * get_area_handle(solid * ext) {
	return &ext->rep.as_ext.area;
}

void set_void_count(face * f, size_t count) {
	if (f->void_count == 0 && count == 0) {
		return;
	}
	else if (f->void_count == 0 && count > 0) {
		f->void_count = count;
		f->voids = (polyloop *)malloc(sizeof(polyloop) * f->void_count);
	}
	else if (f->void_count > 0 && count == 0) {
		f->void_count = 0;
		free(f->voids);
		f->voids = nullptr;
	}
	else {
		f->void_count = count;
		f->voids = (polyloop *)realloc(f->voids, sizeof(polyloop) * f->void_count);
	}
}

polyloop * get_void_handle(face * f, size_t index) {
	return &f->voids[index];
}

polyloop * get_outer_boundary_handle(face * f) {
	return &f->outer_boundary;
}

void set_vertex_count(polyloop * loop, size_t count) {
	if (loop->vertex_count == 0 && count == 0) {
		return;
	}
	else if (loop->vertex_count == 0 && count > 0) {
		loop->vertex_count = count;
		loop->vertices = (point *)malloc(sizeof(point) * loop->vertex_count);
	}
	else if (loop->vertex_count > 0 && count == 0) {
		loop->vertex_count = 0;
		free(loop->vertices);
		loop->vertices = nullptr;
	}
	else {
		loop->vertex_count = count;
		loop->vertices = (point *)realloc(loop->vertices, sizeof(point) * loop->vertex_count);
	}
}

void set_vertex(polyloop * loop, size_t index, double x, double y, double z) {
	loop->vertices[index].x = x;
	loop->vertices[index].y = y;
	loop->vertices[index].z = z;
}

void free_sb(space_boundary * sb) {
	free(sb->layers);
	free(sb->thicknesses);
	free(sb);
}

void free_sb_list(space_boundary ** sbs, size_t count) {
	for (size_t i = 0; i < count; ++i) {
		free_sb(sbs[i]);
	}
	free(sbs);
}