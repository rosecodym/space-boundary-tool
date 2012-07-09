#pragma once

#include "precompiled.h"

#include "area.h"
#include "layer_information.h"

class space;

class blockstack {
private:
	area a;
	std::vector<layer_information> layers;
	std::pair<const space *, boost::optional<const space *>> spaces;
public:
	template <typename LayerRange>
	blockstack(area && a, const LayerRange & layers, const space * sp_a, const space * sp_b = nullptr)
		: a(a), layers(layers.begin(), layers.end()), spaces(sp_a, sp_b == nullptr ? boost::optional<const space *>() : sp_b)
	{ }
};