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
	IFCADAPT_INVALID_ARGS = 3,
	IFCADAPT_UNKNOWN
END(ifcadapter_return_t)

#ifdef SBT_IFC_EXPORTS
#define DLLINEX dllexport
#else
#define DLLINEX dllimport
#endif
__declspec(DLLINEX) ifcadapter_return_t execute(
	const char * input_filename,
	const char * output_filename, // NULL for no write-back
	sb_calculation_options opts,
	size_t * element_count,
	element_info *** elements,
	double ** composite_layer_dxs,
	double ** composite_layer_dys,
	double ** composite_layer_dzs,
	size_t * space_count,
	space_info *** spaces,
	size_t * sb_count,
	space_boundary *** sbs);

__declspec(DLLINEX) void release_elements(element_info ** elements, size_t count);
__declspec(DLLINEX) void release_spaces(space_info ** spaces, size_t count);

#undef DLLINEX

#undef BEGINSTRUCT
#undef BEGINENUM
#undef END
#undef BOOLTYPE
#undef STRUCTTYPE

#ifdef __cplusplus
}
#endif