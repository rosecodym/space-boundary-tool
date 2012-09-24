#pragma once

#include "../../Core/src/sbt-core.h"

#ifdef __cplusplus
#define BOOLTYPE bool
#define STRUCTTYPE(name) name
#define BEGINSTRUCT(name) struct name {
#define BEGINENUM(name) enum name {
#define END(name) };
#else
#define BOOLTYPE int
#define STRUCTTYPE(name) struct name
#define BEGINSTRUCT(name) typedef struct {
#define BEGINENUM(name) typedef enum {
#define END(name) } name;
#endif

#ifdef __cplusplus
extern "C" {
#endif

BEGINENUM(ifcadapter_return_t)
	IFCADAPT_OK = 0,
	IFCADAPT_EDM_ERROR = 1,
	IFCADAPT_TOO_COMPLICATED = 2,
	IFCADAPT_UNKNOWN
END(ifcadapter_return_t)

BEGINSTRUCT(sb_counts)
	size_t bounded_space_count;
	char (*space_guids)[SPACE_ID_MAX_LEN]; // points to a dynamically-allocated array of char[]s
	int * level_2_physical_internal;
	int * level_2_physical_external;
	int * level_3_internal;
	int * level_3_external;
	int * level_4;
	int * level_5;
	int * virt;
END(sb_counts)

#ifdef SBT_IFC_EXPORTS
#define DLLINEX dllexport
#else
#define DLLINEX dllimport
#endif
__declspec(DLLINEX) ifcadapter_return_t add_to_ifc_file(const char * input_filename, const char * output_filename, sb_calculation_options options, sb_counts * counts);
__declspec(DLLINEX) ifcadapter_return_t load_and_run_from(
	const char * input_filename,
	const char * output_filename, // NULL if you don't want to write back
	sb_calculation_options opts,
	element_info *** elements,
	size_t * element_count,
	space_info *** spaces,
	size_t * space_count,
	space_boundary *** space_boundaries,
	size_t * total_sb_count);
__declspec(DLLINEX) void free_sb_counts(sb_counts counts);
__declspec(DLLINEX) void free_elements(element_info ** elements, size_t count);
__declspec(DLLINEX) void free_spaces(space_info ** spaces, size_t count);
__declspec(DLLINEX) void free_space_boundaries(space_boundary ** sbs, size_t count);
#undef DLLINEX

#undef BEGINSTRUCT
#undef BEGINENUM
#undef END
#undef BOOLTYPE
#undef STRUCTTYPE

#ifdef __cplusplus
}
#endif