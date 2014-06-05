#pragma once

#ifndef __cplusplus
#include <stddef.h>
#include <stdio.h>
#else
#include <cstdio>
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Names and such use fixed-width fields, and these are the widths.
#define SB_ID_MAX_LEN 127			// Space boundary IDs
#define SPACE_ID_MAX_LEN 127		// Space IDs
#define ELEMENT_NAME_MAX_LEN 127	// Element names

typedef char sb_id_t[SB_ID_MAX_LEN + 1];
typedef char space_id_t[SPACE_ID_MAX_LEN + 1];

// The difference between an element name and an element ID is half historical
// and half "names are descriptions for humans and IDs are descriptions for
// computers." The name is only used in the SBT core when messages are emitted.
// The ID is what ends up in the "layer list" returned by the core. This scheme
// is kind of annoying because an element name is analogous to a space ID, and
// an element ID is a completely different beast, but we're all going to live
// with it for the moment.
typedef char element_name_t[ELEMENT_NAME_MAX_LEN + 1];
typedef int element_id_t;	

// See the definition for solids below.
enum solid_rep_type {
	REP_NOTHING = 0,
	REP_BREP,
	REP_EXT
};

// The type of an element governs its behavior within the core. The specific
// behavior of each is a little too complicated for a header file comment, but
// it is appropriate to say here that any element with the type UNKNOWN will
// be ignored. (However, clients *shouldn't* rely on this behavior - use the
// filters, described below, to intentionally ignore objects.)
enum element_type {
	WALL = 0,
	SLAB,
	DOOR,
	WINDOW,
	COLUMN,
	BEAM,
	UNKNOWN	
};

enum sbt_return_t {
	SBT_OK = 0,
	SBT_STACK_OVERFLOW = 1,
	SBT_FAILED_ALLOCATION = 2,
	SBT_UNSUPPORTED = 3,
	SBT_UNKNOWN = -1
};

struct point {
	double x;
	double y;
	double z;
};

struct polyloop {
	size_t vertex_count;
	struct point * vertices;
};

struct face {
	struct polyloop outer_boundary;
	size_t void_count;
	struct polyloop * voids;
};

// Note: if a brep face has voids, the voids and all faces directly connected
// to the voids (other than the hosting face) will be ignored.
struct brep {
	size_t face_count;
	struct face * faces;
};

struct extruded_area_solid {
	double ext_dx;
	double ext_dy;
	double ext_dz;
	double extrusion_depth;
	struct face area;
};

struct solid {
	enum solid_rep_type rep_type;
	union {
		struct brep as_brep;
		struct extruded_area_solid as_ext;
	} rep;
};

struct element_info {
	element_name_t name;
	enum element_type type;
	element_id_t id;
	struct solid geometry;
};
	
struct space_info {
	space_id_t id;
	struct solid geometry;
};

struct space_boundary {
	sb_id_t global_id;
	element_name_t element_name;
	struct polyloop geometry;
	double normal_x;
	double normal_y;
	double normal_z;						
	int is_external;
	int is_virtual;
	struct space_info * bounded_space;	
	struct space_boundary * opposite;	
	struct space_boundary * parent;
	size_t material_layer_count;
	element_id_t * layers;
	double * thicknesses;
};

enum sb_options_flags {
	SBT_NONE = 0
};

struct sb_calculation_options {
	int flags;
	double length_units_per_meter;
	double max_pair_distance_in_meters;
	double tolernace_in_meters;
	int unused;
	char ** space_filter;
	size_t space_filter_count;
	char ** element_filter;
	size_t element_filter_count;
	void (*notify_func)(char *);
	void (*warn_func)(char *);
	void (*error_func)(char *);
};

#ifdef SBT_CORE_EXPORTS
#define SBT_CORE_INTERFACE dllexport
#else
#define SBT_CORE_INTERFACE dllimport
#endif

__declspec(SBT_CORE_INTERFACE)
enum sbt_return_t calculate_space_boundaries(
	size_t element_count,						// in
	struct element_info ** elements,			// in
	size_t space_count,							// in
	struct space_info ** spaces,				// in
	size_t * space_boundary_count,				// out
	struct space_boundary *** space_boundaries,	// out
	struct sb_calculation_options opts);		// in

__declspec(SBT_CORE_INTERFACE)
void release_space_boundaries(struct space_boundary ** sbs, size_t count);

__declspec(SBT_CORE_INTERFACE)
struct sb_calculation_options create_default_options(void);

#undef SBT_INTERFACE

#ifdef __cplusplus
}
#endif