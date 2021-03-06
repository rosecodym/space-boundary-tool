#pragma once

#include "../../Core/src/sbt-core.h"

#ifdef __cplusplus
extern "C" {
#endif

enum ifcadapter_return_t {
	IFCADAPT_OK = 0,
	IFCADAPT_IFC_ERROR = 1,
	IFCADAPT_STACK_OVERFLOW = 2,
	IFCADAPT_INVALID_ARGS = 3,
	IFCADAPT_UNKNOWN
};

#ifdef SBT_IFC_EXPORTS
#define SBT_IFC_INTERFACE dllexport
#else
#define SBT_IFC_INTERFACE dllimport
#endif
__declspec(SBT_IFC_INTERFACE) enum ifcadapter_return_t execute(
	char * input_filename,
	char * output_filename, // NULL for no write-back
	struct sb_calculation_options opts,
	size_t * element_count,
	struct element_info *** elements,
	double ** composite_layer_dxs,
	double ** composite_layer_dys,
	double ** composite_layer_dzs,
	size_t * space_count,
	struct space_info *** spaces,
	size_t * sb_count,
	struct space_boundary *** sbs,
	float ** corrected_areas,
	int * totalPoints,
	int * totalEdges,
	int * totalFaces,
	int * totalSolids);

__declspec(SBT_IFC_INTERFACE) 
void release_elements(struct element_info ** elements, size_t count);

__declspec(SBT_IFC_INTERFACE) 
void release_spaces(struct space_info ** spaces, size_t count);

__declspec(SBT_IFC_INTERFACE)
void release_corrected_areas(float * corrected_areas);

#ifdef __cplusplus
}
#endif