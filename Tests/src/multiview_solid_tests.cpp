#include "precompiled.h"

#include <gtest/gtest.h>

#include "common.h"
#include "element.h"
#include "equality_context.h"
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
	solid s;
	set_to_extruded_area_solid(&s, 0, 0, 1, 300);
	face * a = get_area_handle(&s);
	set_void_count(a, 0);
	polyloop * outer = get_outer_boundary_handle(a);
	set_vertex_count(outer, 4);
	set_vertex(outer, 3, 1, 2, 0);
	set_vertex(outer, 2, 10, 2, 0);
	set_vertex(outer, 1, 10, 15, 0);
	set_vertex(outer, 0, 1, 15, 0);
	
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
	solid s;
	set_to_extruded_area_solid(&s, 0, 0, 1, 300);
	face * a = get_area_handle(&s);
	set_void_count(a, 0);
	polyloop * outer = get_outer_boundary_handle(a);
	set_vertex_count(outer, 4);
	set_vertex(outer, 0, 1, 2, 0);
	set_vertex(outer, 1, 10, 2, 0);
	set_vertex(outer, 2, 10, 15, 0);
	set_vertex(outer, 3, 1, 15, 0);
	
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

} // namespace