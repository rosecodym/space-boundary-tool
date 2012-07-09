#pragma once

#include "precompiled.h"

#include "area.h"
#include "layer_information.h"

class space;

class blockstack {
private:
	area a;
	std::vector<layer_information> layers;
	std::pair<space *, boost::optional<space *>> spaces;
public:
	template <typename LayerRange>
	blockstack(area && a, const LayerRange & layers, space * sp_a, space * sp_b = nullptr)
		: a(a), layers(layers.begin(), layers.end()), spaces(sp_a, sp_b == nullptr ? boost::optional<space *>() : sp_b)
	{ }
};