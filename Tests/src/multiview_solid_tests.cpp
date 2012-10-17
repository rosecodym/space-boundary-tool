#include "precompiled.h"

#include <gtest/gtest.h>

#include "common.h"
#include "element.h"
#include "equality_context.h"
#include "exceptions.h"
#include "multiview_solid.h"

namespace {

struct face_information {
	bool * found_flag;
	direction_3 d;
	bool sense;
	point_3 example_point;
	NT p;
	face_information() { }
	face_information(bool * flag, const direction_3 & d, const point_3 & pt, bool sense, const NT & p)
		: found_flag(flag), d(d), example_point(pt), sense(sense), p(p)
	{ }
};

TEST(MultiviewSolid, SimpleExtrusion) {
	equality_context c(0.01);
	solid s = create_ext(0, 0, 1, 300, create_face(4,
		simple_point(1, 15, 0),
		simple_point(10, 15, 0),
		simple_point(10, 2, 0),
		simple_point(1, 2, 0)));
	
	multiview_solid ms(s, &c);
	auto faces = ms.oriented_faces(&c);

	ASSERT_EQ(6, faces.size());

	bool low = false;
	bool high = false;
	bool left = false;
	bool right = false;
	bool front = false;
	bool back = false;

	std::map<NT, face_information> locations;
	locations[0] = face_information(&low, direction_3(0, 0, 1), point_3(5, 5, 0), false, 0);
	locations[300] = face_information(&high, direction_3(0, 0, 1), point_3(5, 5, 300), true, 300);
	locations[1] = face_information(&left, direction_3(1, 0, 0), point_3(1, 8, 50), false, 1);
	locations[10] = face_information(&right, direction_3(1, 0, 0), point_3(10, 8, 50), true, 10);
	locations[2] = face_information(&front, direction_3(0, 1, 0), point_3(4, 2, 100), false, 2);
	locations[15] = face_information(&back, direction_3(0, 1, 0), point_3(4, 15, 100), true, 15);

	boost::for_each(faces, [&locations](const oriented_area & f) {
		NT plane_distance = CGAL::sqrt(CGAL::squared_distance(f.backing_plane(), point_3(0, 0, 0)));
		auto match = locations.find(plane_distance);
		ASSERT_NE(match, locations.end());
		EXPECT_FALSE(*match->second.found_flag) << "Backing plane with distance " << CGAL::to_double(plane_distance);
		*match->second.found_flag = true;
		EXPECT_EQ(match->second.d, f.orientation().direction());
		EXPECT_EQ(match->second.sense, f.sense()) << "Backing plane with distance " << CGAL::to_double(plane_distance);
		EXPECT_EQ(match->second.p, f.height());
		EXPECT_TRUE(f.backing_plane().has_on(match->second.example_point)) << "Backing plane with distance " << CGAL::to_double(plane_distance);
	});
}

TEST(MultiviewSolid, SimpleExtrusionBaseReversed) {
	equality_context c(0.01);
	solid s = create_ext(0, 0, 1, 300, create_face(4,
		simple_point(1, 2, 0),
		simple_point(10, 2, 0),
		simple_point(10, 15, 0),
		simple_point(1, 15, 0)));
	
	multiview_solid ms(s, &c);
	auto faces = ms.oriented_faces(&c);

	ASSERT_EQ(6, faces.size());

	bool low = false;
	bool high = false;
	bool left = false;
	bool right = false;
	bool front = false;
	bool back = false;

	std::map<NT, face_information> locations;
	locations[0] = face_information(&low, direction_3(0, 0, 1), point_3(5, 5, 0), false, 0);
	locations[300] = face_information(&high, direction_3(0, 0, 1), point_3(5, 5, 300), true, 300);
	locations[1] = face_information(&left, direction_3(1, 0, 0), point_3(1, 8, 50), false, 1);
	locations[10] = face_information(&right, direction_3(1, 0, 0), point_3(10, 8, 50), true, 10);
	locations[2] = face_information(&front, direction_3(0, 1, 0), point_3(4, 2, 100), false, 2);
	locations[15] = face_information(&back, direction_3(0, 1, 0), point_3(4, 15, 100), true, 15);

	boost::for_each(faces, [&locations](const oriented_area & f) {
		NT plane_distance = CGAL::sqrt(CGAL::squared_distance(f.backing_plane(), point_3(0, 0, 0)));
		auto match = locations.find(plane_distance);
		ASSERT_NE(match, locations.end());
		EXPECT_FALSE(*match->second.found_flag) << "Backing plane with distance " << CGAL::to_double(plane_distance);
		*match->second.found_flag = true;
		EXPECT_EQ(match->second.d, f.orientation().direction());
		EXPECT_EQ(match->second.sense, f.sense()) << "Backing plane with distance " << CGAL::to_double(plane_distance);
		EXPECT_EQ(match->second.p, f.height());
		EXPECT_TRUE(f.backing_plane().has_on(match->second.example_point)) << "Backing plane with distance " << CGAL::to_double(plane_distance);
	});
}

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

TEST(MultiviewSolid, OpenBrep) {
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

} // namespace