#pragma once

#include "precompiled.h"

class surface;

namespace operations {

std::vector<std::shared_ptr<surface>> assign_openings(const std::vector<std::shared_ptr<surface>> & surfaces);

}