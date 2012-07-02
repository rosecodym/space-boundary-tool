#include "precompiled.h"

#include "poly_builder.h"
#include "printing-macros.h"
#include "simple_face.h"

#include "solid_conversion_operations.h"

namespace solid_geometry {

namespace impl {

nef_polyhedron_3 extrusion_to_nef(const extrusion_information & ext, equality_context * c) {
	const simple_face & f = std::get<0>(ext);
	const vector_3 & extrusion = std::get<1>(ext);

	if (FLAGGED(SBT_VERBOSE_ELEMENTS | SBT_VERBOSE_SPACES)) {
		NOTIFY_MSG("Converting extrusion information (%u inner loops) to a nef polyhedron.\nExtrusion:\n", f.inners().size());
		PRINT_VECTOR_3(extrusion);
		NOTIFY_MSG("\nOuter:\n");
		PRINT_LOOP_3(f.outer());
		NOTIFY_MSG("\n");
		boost::for_each(f.inners(), [](const std::vector<point_3> & inner) { NOTIFY_MSG("Inner:\n"); PRINT_LOOP_3(inner); NOTIFY_MSG("\n"); });
	}
	
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
	PRINT_SOLIDS("Nef polyhedron from extrusion information had %u volume(s).\n", res.number_of_volumes());
	if (FLAGGED(SBT_EXPENSIVE_CHECKS) && res.number_of_volumes() < 2) {
		ERROR_MSG("Nef polyhedron from extrusion information had %u volume(s).\n", res.number_of_volumes());
		abort();
	}
	return res;
}

} // namespace impl

} // namespace impl