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

// defines and typedefs

#define SB_ID_MAX_LEN 127							// maximum length for space boundary ids
#define SPACE_ID_MAX_LEN 127						// maximum length for space ids
#define ELEMENT_ID_MAX_LEN 127						// maximum length for element ids

typedef char sb_id_t[SB_ID_MAX_LEN + 1];			// space boundary id type
typedef char space_id_t[SPACE_ID_MAX_LEN + 1];		// space id type
typedef char element_id_t[ELEMENT_ID_MAX_LEN + 1];	// element id type
typedef int material_id_t;							// material id type. material ids are meaningless within the core;
													// the client is responsible for making sense of them

#ifdef __cplusplus
#define STRUCTTYPE(name) name
#define BEGINSTRUCT(name) struct name {
#define BEGINENUM(name) enum name {
#define END(name) };
#else
#define STRUCTTYPE(name) struct name
#define BEGINSTRUCT(name) typedef struct {
#define BEGINENUM(name) typedef enum {
#define END(name) } name;
#endif

// tag to indicate the representation of a solid
BEGINENUM(solid_rep_type)
	REP_NOTHING = 0,
	REP_BREP,
	REP_EXT
END(solid_rep_type)

// the type of an element governs its interactions with other elements
BEGINENUM(element_type)
	WALL = 0,	// walls have slab and column volumes subtracted from their volume
	SLAB,		// slabs subtract their volume from walls and columns
	DOOR,		// doors will be embedded into walls
	WINDOW,		// windows will be embedded into walls
	COLUMN,		// columns subtract their volumes from walls
	BEAM,		// currently unused
	UNKNOWN		// currently unused
END(element_type)

// this list to be expanded later
BEGINENUM(sbt_return_t)
	SBT_OK = 0,
	SBT_TOO_COMPLICATED = 1,
	SBT_FAILED_ALLOCATION = 2,
	SBT_UNSUPPORTED = 3,
	SBT_UNKNOWN = -1
END(sbt_return_t)

// points
BEGINSTRUCT(point)
	double x;
	double y;
	double z;
END(point)

// polyloops in 3-space
BEGINSTRUCT(polyloop)
	size_t vertex_count;
	point * vertices;
END(polyloop)

// faces (polyloop outer boundary with optional voids)
BEGINSTRUCT(face)
	polyloop outer_boundary;
	size_t void_count;
	polyloop * voids;
END(face)

// very primitive. order doesn't matter.
// faces is a dynamically allocated array
BEGINSTRUCT(brep)
	size_t face_count;
	face * faces;
END(brep)

// less primitive.
BEGINSTRUCT(extruded_area_solid)
	double ext_dx;
	double ext_dy;
	double ext_dz;
	double extrusion_depth;
	face area;
END(extruded_area_solid)

// "supertype" for solid representation
BEGINSTRUCT(solid)
	solid_rep_type rep_type;
	union {
		brep as_brep;
		extruded_area_solid as_ext;
	} rep;
END(solid)

// type for elements
BEGINSTRUCT(element_info)
	element_id_t id;
	element_type type;
	material_id_t material;
	solid geometry;
END(element_info)
	
// type for spaces
BEGINSTRUCT(space_info)
	space_id_t id;
	solid geometry;
END(space_info)

// the belle of the ball
BEGINSTRUCT(space_boundary)
	sb_id_t global_id;
	element_id_t element_id;
	polyloop geometry;
	double normal_x;
	double normal_y;
	double normal_z;						
	int is_external;
	int is_virtual;
	space_info * bounded_space;	
	STRUCTTYPE(space_boundary) * opposite;	
	STRUCTTYPE(space_boundary) * parent;
	size_t material_layer_count;
	material_id_t * layers;
	double * thicknesses;
END(space_boundary)

BEGINENUM(sbt_options_flags)
	SBT_NONE = 0
END(sbt_options_flags)

BEGINSTRUCT(sb_calculation_options)
	int flags;
	double length_units_per_meter;
	double max_pair_distance_in_meters;
	int space_verification_timeout;
	char ** space_filter;
	size_t space_filter_count;
	char ** element_filter;
	size_t element_filter_count;
	void (*notify_func)(char *);
	void (*warn_func)(char *);
	void (*error_func)(char *);
END(sb_calculation_options)

// entry point prototype

#ifdef SBT_CORE_EXPORTS
#define DLLINEX dllexport
#else
#define DLLINEX dllimport
#endif

__declspec(DLLINEX)
sbt_return_t calculate_space_boundaries(
	size_t element_count,					// (in)
	element_info ** elements,				// (in) dynamically allocated array of pointers to elements
	size_t space_count,						// (in)
	space_info ** spaces,					// (in) dynamically allocated array of pointers to spaces
	size_t * space_boundary_count,			// (out)
	space_boundary *** space_boundaries,	// (out) dynamically allocated array of pointers to space boundaries
	sb_calculation_options opts);

__declspec(DLLINEX)
void release_space_boundaries(space_boundary ** sbs, size_t count);

__declspec(DLLINEX)
sb_calculation_options create_default_options(void);

#undef DLLINEX

#undef BEGINSTRUCT
#undef BEGINENUM
#undef END
#undef STRUCTTYPE

#ifdef __cplusplus
}
#endif