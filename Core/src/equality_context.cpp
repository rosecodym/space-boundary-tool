#include "precompiled.h"

#include "geometry_common.h"
#include "sbt-core.h"

#include "equality_context.h"

extern sb_calculation_options g_opts;
extern char g_msgbuf[256];

void equality_context::init_constants()
{
	heights.request(0.0); heights.request(1.0);
	xs_2d.request(0.0); xs_2d.request(1.0);
	ys_2d.request(0.0); ys_2d.request(1.0);
	xs_3d.request(0.0); xs_3d.request(1.0);
	ys_3d.request(0.0); ys_3d.request(1.0);
	zs_3d.request(0.0); zs_3d.request(1.0);
	snap(direction_3(1.0, 0.0, 0.0));
	snap(direction_3(0.0, 1.0, 0.0));
	snap(direction_3(0.0, 0.0, 1.0));
	request_orientation(direction_3(1, 0, 0));
	request_orientation(direction_3(0, 1, 0));
	request_orientation(direction_3(0, 0, 1));
}

direction_3 equality_context::snap(const direction_3 & d) {
	vector_3 v = geometry_common::normalize(d.to_vector());
	for (auto p = directions.begin(); p != directions.end(); ++p) {
		if (p->first == d) {
			return d;
		}
		if (is_zero_squared(CGAL::cross_product(p->second, v).squared_length(), tolerance / 100)) {
			if (geometry_common::share_sense(d, p->first)) {
				return p->first;
			}
			else {
				return -p->first;
			}
		}
	}
	directions.push_back(std::make_pair(d, v));
	return d;
}

polygon_2 equality_context::snap(const polygon_2 & poly) {
	polygon_2 res;
	for (auto p = poly.vertices_begin(); p != poly.vertices_end(); ++p) {
		res.push_back(this->snap(*p));
	}
	return res.is_simple() ? res : polygon_2();
}

std::tuple<orientation *, bool> equality_context::request_orientation(const direction_3 & d) {
	auto exists = boost::find_if(orientations, [&d, this](const std::unique_ptr<orientation> & o) {
		return are_effectively_parallel(o->direction(), d);
	});
	if (exists != orientations.end()) {
		return std::make_tuple(exists->get(), geometry_common::share_sense((*exists)->direction(), d));
	}
	else {
		orientations.push_back(std::unique_ptr<orientation>(new orientation(d)));
		return std::make_tuple(orientations.back().get(), true);
	}
}