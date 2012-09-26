#include "precompiled.h"

#include "poly_builder.h"
#include "simple_face.h"

#include "solid_conversion_operations.h"

namespace solid_geometry {

namespace impl {

nef_polyhedron_3 extrusion_to_nef(const extrusion_information & ext, equality_context * c) {
	const simple_face & f = std::get<0>(ext);
	const vector_3 & extrusion = std::get<1>(ext);
	
	transformation_3 extrude(CGAL::TRANSLATION, extrusion);
	polyhedron_3 poly;
	poly_builder builder(f.outer(), extrude, c);
	poly.delegate(builder);
	nef_polyhedron_3 res(poly);
	boost::for_each(f.inners(), [&res, &extrude, c](const std::vector<point_3> & inner) {
		polyhedron_3 poly;
		poly_builder builder(inner, extrude, c);
		poly.delegate(builder);
		res -= nef_polyhedron_3(poly);
	});
	return res;
}

} // namespace impl

} // namespace impl