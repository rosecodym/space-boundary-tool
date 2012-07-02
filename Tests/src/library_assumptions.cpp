#include "precompiled.h"

#include <gtest/gtest.h>

namespace {

TEST(LibraryAssumptions, BBox2Construction) {
	boost::optional<bbox_2> res;

	point_2 pts[] = {
		point_2(10, -300),
		point_2(8250, -300),
		point_2(8250, 10),
		point_2(10, 10)
	};

	ASSERT_FALSE(res);

	if (res) {
		res = *res + pts[0].bbox();
	}
	else {
		res = pts[0].bbox();
	}

	EXPECT_DOUBLE_EQ(10, res->xmin());
	EXPECT_DOUBLE_EQ(10, res->xmax());
	EXPECT_DOUBLE_EQ(-300, res->ymin());
	EXPECT_DOUBLE_EQ(-300, res->ymax());

	ASSERT_TRUE(res);

	if (res) {
		res = *res + pts[1].bbox();
	}
	else {
		res = pts[1].bbox();
	}

	EXPECT_DOUBLE_EQ(10, res->xmin());
	EXPECT_DOUBLE_EQ(8250, res->xmax());
	EXPECT_DOUBLE_EQ(-300, res->ymin());
	EXPECT_DOUBLE_EQ(-300, res->ymax());

	ASSERT_TRUE(res);

	if (res) {
		res = *res + pts[2].bbox();
	}
	else {
		res = pts[2].bbox();
	}

	EXPECT_DOUBLE_EQ(10, res->xmin());
	EXPECT_DOUBLE_EQ(8250, res->xmax());
	EXPECT_DOUBLE_EQ(-300, res->ymin());
	EXPECT_DOUBLE_EQ(10, res->ymax());

	ASSERT_TRUE(res);

	if (res) {
		res = *res + pts[3].bbox();
	}
	else {
		res = pts[3].bbox();
	}

	EXPECT_DOUBLE_EQ(10, res->xmin());
	EXPECT_DOUBLE_EQ(8250, res->xmax());
	EXPECT_DOUBLE_EQ(-300, res->ymin());
	EXPECT_DOUBLE_EQ(10, res->ymax());
}

TEST(LibraryAssumptions, PointInHalfspace) {
	plane_3 pl(point_3(0, 0, 0), direction_3(0, 0, 1));
	EXPECT_TRUE(pl.has_on_positive_side(point_3(0, 0, 1)));
	EXPECT_FALSE(pl.has_on_positive_side(point_3(0, 0, 0)));
}

} // namespace