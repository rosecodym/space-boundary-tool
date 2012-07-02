#pragma once

#ifdef SOLIDSPLITTER_EXPORTS
#define DLLINEX dllexport
#else
#define DLLINEX dllimport
#endif

extern "C" {

enum solid_split_return_t {
	SOLID_SPLIT_OK =							0,
	SOLID_SPLIT_POLYLOOP_MESSY =				1,
	SOLID_SPLIT_POLYLOOP_MESSY_NO_INITIAL =		1 | (1 << 16),
	SOLID_SPLIT_POLYLOOP_MESSY_DOWN_TO_TWO =	1 | (2 << 16),
	SOLID_SPLIT_BAD_INPUT =						2,
	SOLID_SPLIT_BAD_INPUT_VOIDS =				2 | (1 << 16),
	SOLID_SPLIT_BAD_INPUT_NON_TRANSITIVE =		2 | (2 << 16),
	SOLID_SPLIT_BAD_INPUT_NO_BASE_PLANE =		2 | (3 << 16)
};

struct solid;

__declspec(DLLINEX) solid_split_return_t split_solid(
	solid s,
	double along_x,
	double along_y, 
	double along_z,
	double thicknesses[],
	int layer_count,
	solid results[]);

__declspec(DLLINEX) void free_created_solids(
	solid solids[],
	int count);

}