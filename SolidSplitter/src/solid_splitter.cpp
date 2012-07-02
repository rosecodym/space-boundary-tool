#include "precompiled.h"

#include "../../Core/src/sbt-core.h"

#include "cgal-typedefs.h"
#include "easy_ext_case.h"
#include "equality_context.h"
#include "exceptions.h"
#include "free.h"
#include "slice.h"

#include "solid_splitter.h"

namespace {

const double TOLERANCE = 0.001;

bool are_oriented_similarly(const vector_3 & a, const vector_3 & b) {
	return
		sign(a.x()) == sign(b.x()) &&
		sign(a.y()) == sign(b.y()) &&
		sign(a.z()) == sign(b.z());
}

vector_3 create_normalized_vector(double dx, double dy, double dz) {
	double mag = sqrt(dx * dx + dy * dy + dz * dz);
	return vector_3(dx / mag, dy / mag, dz / mag);
}

} // namespace

solid_split_return_t split_solid(solid s, double along_dx, double along_dy, double along_dz, double thicknesses[], int layer_count, solid results[]) {
	try {
		equality_context c(TOLERANCE);
		vector_3 layers_vec = create_normalized_vector(c.is_zero(along_dx) ? 0 : along_dx, c.is_zero(along_dy) ? 0 : along_dy, c.is_zero(along_dz) ? 0 : along_dz);
		if (s.rep_type == REP_EXT) {
			vector_3 ext_vec = create_normalized_vector(s.rep.as_ext.ext_dx, s.rep.as_ext.ext_dy, s.rep.as_ext.ext_dz);
			if (c.are_effectively_parallel(layers_vec, ext_vec)) {
				if (!are_oriented_similarly(layers_vec, ext_vec)) {
					std::reverse(thicknesses, thicknesses + layer_count);
				}
				handle_easy_ext_case(s.rep.as_ext, layers_vec, thicknesses, layer_count, results);
				if (!are_oriented_similarly(layers_vec, ext_vec)) {
					std::reverse(thicknesses, thicknesses + layer_count);
					std::reverse(results, results + layer_count);
				}
				return SOLID_SPLIT_OK;
			}
		}
		slice(s, layers_vec, thicknesses, layer_count, results, &c);
		return SOLID_SPLIT_OK;
	}
	catch (polyloop_clean_failure_exception & e) {
		return static_cast<solid_split_return_t>(SOLID_SPLIT_POLYLOOP_MESSY | (e.reason << 16));
	}
	catch (bad_input_exception & e) {
		return static_cast<solid_split_return_t>(SOLID_SPLIT_BAD_INPUT | (e.reason << 16));
	}
}

void free_created_solids(solid solids[], int count) {
	for (int i = 0; i < count; ++i) {
		free_interior(solids[i]);
	}
}