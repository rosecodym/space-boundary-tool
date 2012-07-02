#pragma once

#include "precompiled.h"

#include "cgal-typedefs.h"
#include "sbt-ifcadapter.h"

struct exact_point {
	NT x;
	NT y;
	NT z;
	exact_point(NT x, NT y, NT z) : x(x), y(y), z(z) { }
	void populate_inexact_version(point * p) const;
};

struct exact_polyloop {
	std::vector<exact_point> vertices;
	void populate_inexact_version(polyloop * p) const;
};

struct exact_face {
	exact_polyloop outer_boundary;
	std::vector<exact_polyloop> voids;
	void populate_inexact_version(face * f) const;
};

struct exact_brep {
	std::vector<exact_face> faces;
	void populate_inexact_version(brep * b) const;
};

struct exact_extruded_area_solid {
	exact_face area;
	direction_3 ext_dir;
	NT extrusion_depth;
	void populate_inexact_version(extruded_area_solid * e) const;
};

struct exact_solid {
private:
	solid_rep_type m_rep_type;
public:
	solid_rep_type rep_type() const { return m_rep_type; }
	union {
		exact_brep * as_brep; // i would do by value but union members have to be PODs
		exact_extruded_area_solid * as_ext;
	} rep;
	exact_solid() : m_rep_type(REP_NOTHING) { }
	~exact_solid() { if (m_rep_type == REP_BREP) { delete rep.as_brep; } else if (m_rep_type == REP_EXT) { delete rep.as_ext; } }
	void set_rep_type(solid_rep_type);
	void populate_inexact_version(solid * s) const;
};

inline bool operator == (const exact_point & lhs, const exact_point & rhs) {
	return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
}