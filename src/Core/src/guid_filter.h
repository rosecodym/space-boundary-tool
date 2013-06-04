#pragma once

#include "precompiled.h"

typedef std::function<bool(char *)> guid_filter;

guid_filter create_guid_filter(char ** guids, size_t count);