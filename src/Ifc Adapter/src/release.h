#pragma once

#include "precompiled.h"

struct element_info;
struct space_info;

void release(element_info * e);
void release(space_info * s);

template <typename T>
void release_list(T ** list, size_t count) {
	for (size_t i = 0; i < count; ++i) {
		release(list[i]);
	}
	free(list);
}