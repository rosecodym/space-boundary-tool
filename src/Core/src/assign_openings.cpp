#include "precompiled.h"

#include "assign_openings.h"

#include "surface.h"
#include "oriented_area.h"
#include "area.h"

namespace opening_assignment {

namespace impl {

boost::optional<oriented_area> has_subarea(
	const oriented_area & parent,
	const oriented_area & child,
	double height_eps)
{
	if (parent.sense() == child.sense() &&
		&parent.orientation() == &child.orientation() &&
		equality_context::are_equal(
			parent.height(), 
			child.height(), 
			height_eps))
	{
		auto intr = parent.area_2d() * child.area_2d();
		if (!intr.is_empty()) {
			return oriented_area(parent, intr);
		}
	}
	return boost::optional<oriented_area>();
}

} // namespace impl

} // namespace opening_assignment