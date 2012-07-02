#include "precompiled.h"

#include "cgal-typedefs.h"

#include "number_collection.h"

void number_collection::init_constants()
{
	heights.request(0.0); heights.request(1.0);
	xs_2d.request(0.0); xs_2d.request(1.0);
	ys_2d.request(0.0); ys_2d.request(1.0);
	xs_3d.request(0.0); xs_3d.request(1.0);
	ys_3d.request(0.0); ys_3d.request(1.0);
	zs_3d.request(0.0); zs_3d.request(1.0);
}