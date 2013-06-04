#include "precompiled.h"

#include "common.h"

#include "element.h"
#include "equality_context.h"
#include "exceptions.h"
#include "multiview_solid.h"

namespace {

class MultiviewSolidExtrusionReversed : public ::testing::Test {
protected:
	equality_context c;
	std::vector<oriented_area> faces;
	MultiviewSolidExtrusionReversed() : c(0.01) {
		multiview_solid ms(create_ext(0, 0, 1, 300, create_face(4,
			simple_point(1, 15, 0),
			simple_point(10, 15, 0),
			simple_point(10, 2, 0),
			simple_point(1, 2, 0))), &c);
		faces = ms.oriented_faces(&c);
	}
};

TEST_F(MultiviewSolidExtrusionReversed, HasCorrectFaceCount) {
	EXPECT_EQ(6, faces.size());
}

TEST_F(MultiviewSolidExtrusionReversed, BaseIsCorrect) {
	auto base = boost::find_if(faces, [](const oriented_area & o) {
		return o.height() == 0.0;
	});
	ASSERT_NE(faces.end(), base);
	EXPECT_EQ(direction_3(0, 0, 1), base->orientation().direction());
	EXPECT_FALSE(base->sense());
}

TEST_F(MultiviewSolidExtrusionReversed, TargetIsCorrect) {
	auto target = boost::find_if(faces, [](const oriented_area & o) {
		return o.height() == 300;
	});
	ASSERT_NE(faces.end(), target);
	EXPECT_EQ(direction_3(0, 0, 1), target->orientation().direction());
	EXPECT_TRUE(target->sense());
}

TEST_F(MultiviewSolidExtrusionReversed, LeftIsCorrect) {
	auto side = boost::find_if(faces, [](const oriented_area & o) {
		return o.height() == 1;
	});
	ASSERT_NE(faces.end(), side);
	EXPECT_EQ(direction_3(1, 0, 0), side->orientation().direction());
	EXPECT_FALSE(side->sense());
}

TEST_F(MultiviewSolidExtrusionReversed, RightIsCorrect) {
	auto side = boost::find_if(faces, [](const oriented_area & o) {
		return o.height() == 10;
	});
	ASSERT_NE(faces.end(), side);
	EXPECT_EQ(direction_3(1, 0, 0), side->orientation().direction());
	EXPECT_TRUE(side->sense());
}

TEST_F(MultiviewSolidExtrusionReversed, FrontIsCorrect) {
	auto side = boost::find_if(faces, [](const oriented_area & o) {
		return o.height() == 2;
	});
	ASSERT_NE(faces.end(), side);
	EXPECT_EQ(direction_3(0, 1, 0), side->orientation().direction());
	EXPECT_FALSE(side->sense());
}

TEST_F(MultiviewSolidExtrusionReversed, BackIsCorrect) {
	auto side = boost::find_if(faces, [](const oriented_area & o) {
		return o.height() == 15;
	});
	ASSERT_NE(faces.end(), side);
	EXPECT_EQ(direction_3(0, 1, 0), side->orientation().direction());
	EXPECT_TRUE(side->sense());
}

class MultiviewSolidExtrusionAgrees : public ::testing::Test {
protected:
	equality_context c;
	std::vector<oriented_area> faces;
	MultiviewSolidExtrusionAgrees() : c(0.01) {
		multiview_solid ms(create_ext(0, 0, 1, 300, create_face(4,
			simple_point(1, 2, 0),
			simple_point(10, 2, 0),
			simple_point(10, 15, 0),
			simple_point(1, 15, 0))), &c);
		faces = ms.oriented_faces(&c);
	}
};

TEST_F(MultiviewSolidExtrusionAgrees, HasCorrectFaceCount) {
	EXPECT_EQ(6, faces.size());
}

TEST_F(MultiviewSolidExtrusionAgrees, BaseIsCorrect) {
	auto base = boost::find_if(faces, [](const oriented_area & o) {
		return o.height() == 0.0;
	});
	ASSERT_NE(faces.end(), base);
	EXPECT_EQ(direction_3(0, 0, 1), base->orientation().direction());
	EXPECT_FALSE(base->sense());
}

TEST_F(MultiviewSolidExtrusionAgrees, TargetIsCorrect) {
	auto target = boost::find_if(faces, [](const oriented_area & o) {
		return o.height() == 300;
	});
	ASSERT_NE(faces.end(), target);
	EXPECT_EQ(direction_3(0, 0, 1), target->orientation().direction());
	EXPECT_TRUE(target->sense());
}

TEST_F(MultiviewSolidExtrusionAgrees, LeftIsCorrect) {
	auto side = boost::find_if(faces, [](const oriented_area & o) {
		return o.height() == 1;
	});
	ASSERT_NE(faces.end(), side);
	EXPECT_EQ(direction_3(1, 0, 0), side->orientation().direction());
	EXPECT_FALSE(side->sense());
}

TEST_F(MultiviewSolidExtrusionAgrees, RightIsCorrect) {
	auto side = boost::find_if(faces, [](const oriented_area & o) {
		return o.height() == 10;
	});
	ASSERT_NE(faces.end(), side);
	EXPECT_EQ(direction_3(1, 0, 0), side->orientation().direction());
	EXPECT_TRUE(side->sense());
}

TEST_F(MultiviewSolidExtrusionAgrees, FrontIsCorrect) {
	auto side = boost::find_if(faces, [](const oriented_area & o) {
		return o.height() == 2;
	});
	ASSERT_NE(faces.end(), side);
	EXPECT_EQ(direction_3(0, 1, 0), side->orientation().direction());
	EXPECT_FALSE(side->sense());
}

TEST_F(MultiviewSolidExtrusionAgrees, BackIsCorrect) {
	auto side = boost::find_if(faces, [](const oriented_area & o) {
		return o.height() == 15;
	});
	ASSERT_NE(faces.end(), side);
	EXPECT_EQ(direction_3(0, 1, 0), side->orientation().direction());
	EXPECT_TRUE(side->sense());
}

TEST(MultiviewSolidSharePlaneOpposite, EmbeddedColumn) {
	equality_context c(0.01);
	multiview_solid col(create_ext(0, 0, 1, 108, create_face(4,
		simple_point(4, 1, 0),
		simple_point(4, 5, 0),
		simple_point(6, 5, 0),
		simple_point(6, 1, 0))), &c);
	multiview_solid wall(create_ext(0, 0, 1, 84, create_face(4,
		simple_point(8, 3, 0),
		simple_point(2, 3, 0),
		simple_point(2, 5, 0),
		simple_point(8, 5, 0))), &c);
	EXPECT_FALSE(multiview_solid::share_plane_opposite(col, wall, &c));
}

TEST(MultiviewSolidSubtract, EmbeddedColumnYieldsMultipleVolumes) {
	equality_context c(0.01);
	multiview_solid col(create_ext(0, 0, 1, 108, create_face(4,
		simple_point(4, 1, 0),
		simple_point(4, 5, 0),
		simple_point(6, 5, 0),
		simple_point(6, 1, 0))), &c);
	multiview_solid wall(create_ext(0, 0, 1, 84, create_face(4,
		simple_point(8, 3, 0),
		simple_point(2, 3, 0),
		simple_point(2, 5, 0),
		simple_point(8, 5, 0))), &c);
	wall.subtract(col, &c);
	EXPECT_FALSE(wall.is_single_volume());
}

TEST(MultiviewSolidSubtract, WorksAfterFaceExtraction) {
	equality_context c(0.01);
	multiview_solid col(create_ext(0, 0, 1, 108, create_face(4,
		simple_point(4, 1, 0),
		simple_point(4, 5, 0),
		simple_point(6, 5, 0),
		simple_point(6, 1, 0))), &c);
	multiview_solid wall(create_ext(0, 0, 1, 84, create_face(4,
		simple_point(8, 3, 0),
		simple_point(2, 3, 0),
		simple_point(2, 5, 0),
		simple_point(8, 5, 0))), &c);
	wall.oriented_faces(&c);
	wall.subtract(col, &c);
	EXPECT_FALSE(wall.is_single_volume());
}

// legacy tests follow

TEST(MultiviewSolid, ExtrusionWithDuplicateBasePoint) {
	equality_context c(0.01);

	solid s = create_ext(0, 0, 1, 307.08661, create_face(5,
		simple_point(0, 0, 0),
		simple_point(393.70079, 0, 0),
		simple_point(393.70079, 387.79528, 0),
		simple_point(0, 387.79528, 0),
		simple_point(0, 0, 0)));

	multiview_solid ms(s, &c);
	EXPECT_EQ(6, ms.oriented_faces(&c).size());
}

TEST(MultiviewSolid, ThreeStairs) {
	equality_context c(0.01);

	solid s = create_ext(0, 0, 1, 300, create_face(8,
		simple_point(0, 0, 0),
		simple_point(0, 8250, 0),
		simple_point(2105, 8250, 0),
		simple_point(2105, 12120.109, 0),
		simple_point(4050, 12120.109, 0),
		simple_point(4050, 18195.109, 0),
		simple_point(8200, 18195.109, 0),
		simple_point(8200, 0, 0)));

	multiview_solid ms(s, &c);
	EXPECT_EQ(10, ms.oriented_faces(&c).size());
}

TEST(MultiviewSolid, MismatchedFaces) {
	equality_context c(0.01);

	solid s = create_brep(4,
		create_face(3, 
			simple_point(0, 0, 0),
			simple_point(1, 0, 0),
			simple_point(0, 1, 0)),
		create_face(3,
			simple_point(0, 0, 0),
			simple_point(0, 0, 1),
			simple_point(0, 1, 0)),
		create_face(3,
			simple_point(0, 0, 0),
			simple_point(0, 0, 1),
			simple_point(1, 0, 0)),
		create_face(3,
			simple_point(0, 0, 1),
			simple_point(0, 1, 0),
			simple_point(1, 0, 0)));

	multiview_solid mvs(s, &c);
	std::vector<oriented_area> faces = mvs.oriented_faces(&c);
	EXPECT_EQ(4, faces.size());
	EXPECT_TRUE(faces.end() != boost::find_if(faces, [](const oriented_area & o) {
		return o.orientation().direction() == direction_3(0, 0, 1) && !o.sense();
	}));
	EXPECT_TRUE(faces.end() != boost::find_if(faces, [](const oriented_area & o) {
		return o.orientation().direction() == direction_3(0, 1, 0) && !o.sense();
	}));
	EXPECT_TRUE(faces.end() != boost::find_if(faces, [](const oriented_area & o) {
		return o.orientation().direction() == direction_3(1, 0, 0) && !o.sense();
	}));
	EXPECT_TRUE(faces.end() != boost::find_if(faces, [](const oriented_area & o) {
		return o.orientation().direction() == direction_3(1, 1, 1);
	}));
}

TEST(MultiviewSolid, AllObtuseAngleBrep) {
	equality_context c(0.01);

	simple_point points[] = {
		simple_point(0, 0, 0),
		simple_point(2, 0, 1),
		simple_point(1, 1, 1),
		simple_point(-1, 1, 1),
		simple_point(-2, 0, 1),
		simple_point(-1, -1, 1),
		simple_point(1, -1, 1),
		simple_point(0, 0, 2)
	};

	solid s = create_brep(12,
		create_face(3, points[0], points[1], points[2]),
		create_face(3, points[0], points[2], points[3]),
		create_face(3, points[0], points[3], points[4]),
		create_face(3, points[0], points[4], points[5]),
		create_face(3, points[0], points[5], points[6]),
		create_face(3, points[0], points[6], points[1]),
		create_face(3, points[7], points[1], points[2]),
		create_face(3, points[7], points[2], points[3]),
		create_face(3, points[7], points[3], points[4]),
		create_face(3, points[7], points[4], points[5]),
		create_face(3, points[7], points[5], points[6]),
		create_face(3, points[7], points[6], points[1]));

	EXPECT_NO_THROW(multiview_solid mvs(s, &c));
}

TEST(MultiviewSolid, SimplifiedToOpenBrep) {
	equality_context c(0.01);

	solid s = create_brep(10,
		// base/out
		create_face(4,
			simple_point(1.5, 2.8333333333333, 0),
			simple_point(1.5, 0, 0),
			simple_point(0, 0, 0),
			simple_point(0, 2.83333333333333, 0)),
		// ?/out
		create_face(6,
			simple_point(1.5, 0, 0),
			simple_point(1.5, 2.8333333333333333, 0),
			simple_point(1.5, 2.8333333333333333, 10.666666666666),
			simple_point(1.5, 2.826055508613251, 10.666666666666),
			simple_point(1.5, 2.826055508613251, 10.5),
			simple_point(1.5, 0, 10.5)),
		// front short side/out
		create_face(4,
			simple_point(0, 0, 0),
			simple_point(1.5, 0, 0),
			simple_point(1.5, 0, 10.5),
			simple_point(0, 0, 10.5)),
		// ?/in
		create_face(6,
			simple_point(0, 2.833333333333, 0),
			simple_point(0, 2.833333333333, 10.666666666666),
			simple_point(0, 1.251142620394262, 10.666666666666666),
			simple_point(0, 1.251142620394262, 10.5),
			simple_point(0, 0, 10.5),
			simple_point(0, 0, 0)),
		// back short side/in
		create_face(4,
			simple_point(1.5, 2.8333333333, 0),
			simple_point(1.5, 2.8333333333, 10.666666666666),
			simple_point(0, 2.8333333333, 10.666666666666),
			simple_point(0, 2.8333333333, 0)),
		// lip top (rejected)
		create_face(6,
			simple_point(1.5, 2.83333333333333, 10.6666666666),
			simple_point(0, 2.83333333333333, 10.6666666666),
			simple_point(0, 1.251142620394262, 10.6666666666),
			simple_point(0.003271982629936332, 1.251142620394262, 10.6666666666),
			simple_point(0.003271982629939885, 2.826055508613251, 10.6666666666),
			simple_point(1.5, 2.826055508613251, 10.6666666666)),
		// ?
		create_face(6,
			simple_point(0.003271982629939885, 2.826055508613251, 10.5),
			simple_point(1.5, 2.826055508613251, 10.5),
			simple_point(1.5, 0, 10.5),
			simple_point(0, 0, 10.5),
			simple_point(0, 1.251142620394262, 10.5),
			simple_point(0.003271982629936332, 1.251142620394262, 10.5)),
		// half lip edge (rejected)
		create_face(4,
			simple_point(0.003271982629936332, 1.251142620394262, 10.5),
			simple_point(0.003271982629936332, 1.251142620394262, 10.6666666666),
			simple_point(0, 1.251142620394262, 10.6666666666),
			simple_point(0, 1.251142620394262, 10.5)),
		// half lip inside
		create_face(4,
			simple_point(0.003271982629936332, 1.251142620394262, 10.5),
			simple_point(0.003271982629939885, 2.826055508613251, 10.5),
			simple_point(0.003271982629939885, 2.826055508613251, 10.6666666666),
			simple_point(0.003271982629936332, 1.251142620394262, 10.6666666666)),
		// full lip inside
		create_face(4,
			simple_point(0.003271982629939885, 2.826055508613251, 10.6666666666),
			simple_point(0.003271982629939885, 2.826055508613251, 10.5),
			simple_point(1.5, 2.826055508613251, 10.5),
			simple_point(1.5, 2.826055508613251, 10.6666666666)));
			
	EXPECT_THROW(multiview_solid mvs(s, &c);, bad_brep_exception);
}

TEST(MultiviewSolid, TooShallowExtrusion) {
	equality_context c(0.01);

	solid s = create_ext(0, 0, 1, 0.003, create_face(5,
		simple_point(0, 0, 0),
		simple_point(393.70079, 0, 0),
		simple_point(393.70079, 387.79528, 0),
		simple_point(0, 387.79528, 0),
		simple_point(0, 0, 0)));

	EXPECT_THROW(multiview_solid ms(s, &c);, shallow_extrusion_exception);
}

TEST(MultiviewSolid, BrepsWithArtificialFaceSplitsSucceed) {
	// This tests a stunt that Revit's been pulling recently. Imagine a
	// tetrahedral "tent" with one of the faces split like "flaps." If there's
	// no corresponding vertex at the split on the "floor" face then CGAL will
	// consider the resulting polyhedron open. (In addition, even when this
	// splitting happens to work out space faces get all messed up.)

	equality_context c(0.01);

	solid b = create_brep(5,
		create_face(3,
			simple_point(-1, 0, 0),
			simple_point(0, 1, 0),
			simple_point(1, 0, 0)),
		create_face(3,
			simple_point(-1, 0, 0),
			simple_point(0, 0, 1),
			simple_point(0, 1, 0)),
		create_face(3,
			simple_point(0, 0, 1),
			simple_point(1, 0, 0),
			simple_point(0, 1, 0)),
		create_face(3,
			simple_point(-1, 0, 0),
			simple_point(0, 0, 0),
			simple_point(0, 0, 1)),
		create_face(3,
			simple_point(0, 0, 0),
			simple_point(1, 0, 0),
			simple_point(0, 0, 1)));
			
	EXPECT_NO_THROW(multiview_solid mvs(b, &c););
}

} // namespace