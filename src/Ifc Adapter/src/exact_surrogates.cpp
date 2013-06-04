#include "precompiled.h"

#include "exact_surrogates.h"

void exact_point::populate_inexact_version(point * p) const {
	p->x = CGAL::to_double(x);
	p->y = CGAL::to_double(y);
	p->z = CGAL::to_double(z);
}

void exact_polyloop::populate_inexact_version(polyloop * p) const {
	p->vertex_count = vertices.size();
	p->vertices = (point *)malloc(sizeof(point) * p->vertex_count);
	for (size_t i = 0; i < p->vertex_count; ++i) {
		vertices[i].populate_inexact_version(&p->vertices[i]);
	}
}

void exact_face::populate_inexact_version(face * f) const {
	outer_boundary.populate_inexact_version(&f->outer_boundary);
	f->void_count = voids.size();
	f->voids = (polyloop *)malloc(sizeof(polyloop) * f->void_count);
	for (size_t i = 0; i < f->void_count; ++i) {
		voids[i].populate_inexact_version(&f->voids[i]);
	}
}

void exact_brep::populate_inexact_version(brep * b) const {
	b->face_count = faces.size();
	b->faces = (face *)malloc(sizeof(face) * b->face_count);
	for (size_t i = 0; i < b->face_count; ++i) {
		faces[i].populate_inexact_version(&b->faces[i]);
	}
}

void exact_extruded_area_solid::populate_inexact_version(extruded_area_solid * e) const {
	area.populate_inexact_version(&e->area);
	e->ext_dx = CGAL::to_double(ext_dir.dx());
	e->ext_dy = CGAL::to_double(ext_dir.dy());
	e->ext_dz = CGAL::to_double(ext_dir.dz());
	e->extrusion_depth = CGAL::to_double(extrusion_depth);
}

void exact_solid::set_rep_type(solid_rep_type t) {
	if (rep_type() == REP_BREP) {
		delete rep.as_brep;
	}
	else if (rep_type() == REP_EXT) {
		delete rep.as_ext;
	}
	m_rep_type = t;
	if (rep_type() == REP_BREP) {
		rep.as_brep = new exact_brep();
	}
	else if (rep_type() == REP_EXT) {
		rep.as_ext = new exact_extruded_area_solid();
	}
}

void exact_solid::populate_inexact_version(solid * s) const {
	if (rep_type() == REP_BREP) {
		s->rep_type = REP_BREP;
		rep.as_brep->populate_inexact_version(&s->rep.as_brep);
	}
	else if (rep_type() == REP_EXT) {
		s->rep_type = REP_EXT;
		rep.as_ext->populate_inexact_version(&s->rep.as_ext);
	}
	else {
		s->rep_type = REP_NOTHING;
	}
}