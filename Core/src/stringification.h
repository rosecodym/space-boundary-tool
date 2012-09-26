#pragma once

#include "precompiled.h"

namespace reporting {

std::string to_string(const point_2 & p);
std::string to_string(const point_3 & p);
std::string to_string(const direction_3 & d);
std::string to_string(const vector_3 & v);

template <typename StringifiableRange>
std::string to_string(const StringifiableRange & points) {
	std::stringstream ss;
	boost::for_each(points, [&ss](const StringifiableRange::value_type & s) { ss << to_string(s) << std::endl; });
	return ss.str();
}

std::string to_string(const polygon_2 & poly);

} // namespace reporting