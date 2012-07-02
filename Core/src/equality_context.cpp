#include "precompiled.h"

#include "cgal-util.h"
#include "misc-util.h"
#include "printing-util.h"
#include "sbt-core.h"

#include "equality_context.h"

#define PRINT_GEOM(...) \
	do { \
		if (FLAGGED(SBT_VERBOSE_GEOMETRY)) { \
			sprintf(g_msgbuf, __VA_ARGS__); \
			g_opts.notify_func(g_msgbuf); \
		} \
	} \
	while (false);

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
	request_orientation(direction_3(1, 0, 0));
	request_orientation(direction_3(0, 1, 0));
	request_orientation(direction_3(0, 0, 1));
}

direction_3 equality_context::snap(const direction_3 & d) {
	vector_3 v = util::cgal::normalize(d.to_vector());
	for (auto p = directions.begin(); p != directions.end(); ++p) {
		if (p->first == d) {
			return d;
		}
		if (is_zero_squared(CGAL::cross_product(p->second, v).squared_length(), tolerance / 100)) {
			CGAL::Sign signs_a[] = { CGAL::sign(d.dx()), CGAL::sign(d.dy()), CGAL::sign(d.dz()) };
			CGAL::Sign signs_b[] = { CGAL::sign(p->first.dx()), CGAL::sign(p->first.dy()), CGAL::sign(p->first.dz()) };
			if (signs_a[0] == signs_b[0] &&
				signs_a[1] == signs_b[1] &&
				signs_a[2] == signs_b[2])
			{
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

polygon_2 equality_context::snap(const polygon_2 & p) {
	polygon_2 res;
	std::for_each(p.vertices_begin(), p.vertices_end(), [&res, this](const point_2 & point) { res.push_back(this->snap(point)); });
	return res;
}

std::tuple<orientation *, bool> equality_context::request_orientation(const direction_3 & d) {
	auto exists = boost::find_if(orientations, [&d, this](const std::unique_ptr<orientation> & o) {
		return are_effectively_parallel(o->direction(), d);
	});
	if (exists != orientations.end()) {
		return std::make_tuple(exists->get(), util::cgal::share_sense((*exists)->direction(), d));
	}
	else {
		orientations.push_back(std::unique_ptr<orientation>(new orientation(d)));
		return std::make_tuple(orientations.back().get(), true);
	}
}