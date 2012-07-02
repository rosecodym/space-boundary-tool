#pragma once

#include "sbt-core.h"

#ifdef __cplusplus
extern "C" {
#else
typedef int bool;
#endif

#ifdef SBT_CORE_EXPORTS
#define DLLINEX dllexport
#else
#define DLLINEX dllimport
#endif

__declspec(DLLINEX) sb_calculation_options create_default_options(void);

__declspec(DLLINEX) bool			is_external_sb(space_boundary * sb);

__declspec(DLLINEX) space_info **	create_space_list(size_t count);
__declspec(DLLINEX) element_info **	create_element_list(size_t count);

__declspec(DLLINEX) void			free_space_list(space_info ** list, size_t count);
__declspec(DLLINEX) void			free_element_list(element_info ** list, size_t count);

__declspec(DLLINEX) void			set_space_id(space_info * info, space_id_t id);
__declspec(DLLINEX) void			set_element_id(element_info * info, element_id_t id);
__declspec(DLLINEX)	void			set_element_type(element_info * info, element_type type);
__declspec(DLLINEX) void			set_element_material(element_info * info, material_id_t mat);

__declspec(DLLINEX) solid *			get_space_geometry_handle(space_info * space);
__declspec(DLLINEX) solid *			get_element_geometry_handle(element_info * element);

__declspec(DLLINEX) void			set_to_brep(solid * s, size_t face_count);
__declspec(DLLINEX) void			set_to_extruded_area_solid(solid * s, double dx, double dy, double dz, double depth);

__declspec(DLLINEX) face *			get_face_handle(solid * brep, size_t index);
__declspec(DLLINEX) face *			get_area_handle(solid * ext);

__declspec(DLLINEX) void			set_void_count(face * f, size_t count); // void count defaults to 0
__declspec(DLLINEX) polyloop *		get_void_handle(face * f, size_t index);
__declspec(DLLINEX) polyloop *		get_outer_boundary_handle(face * f);

__declspec(DLLINEX) void			set_vertex_count(polyloop * loop, size_t count);
__declspec(DLLINEX) void			set_vertex(polyloop * loop, size_t index, double x, double y, double z);

__declspec(DLLINEX) void			free_sb(space_boundary * sb);
__declspec(DLLINEX) void			free_sb_list(space_boundary ** sbs, size_t count);

#undef DLLINEX
#undef BOOLTYPE

#ifdef __cplusplus
}
#endif