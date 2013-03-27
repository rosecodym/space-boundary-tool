#include "precompiled.h"

#include <gtest/gtest.h>

#include "equality_context.h"

namespace {

TEST(EqualityContext, NearAxesSnapToAxes) {
	double dx = 0.0;
	double dy = 1.9398725363828362e-006;
	double dz = 1.0000002102065964;
	auto snapped = equality_context(0.01).snap(direction_3(dx, dy, dz));
	EXPECT_EQ(direction_3(0.0, 0.0, 1.0), snapped);
}

} // namespace