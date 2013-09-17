#include "precompiled.h"

#include "../../Edm Wrapper/edm_wrapper_native_interface.h"

#include "sbt-ifcadapter.h"

#include "geometry_common.h"
#include "model_operations.h"
#include "number_collection.h"
#include "reassign_bounded_spaces.h"
#include "release.h"
#include "unit_scaler.h"

sb_calculation_options g_opts;

namespace {

std::function<bool(const char *)> create_guid_filter(char ** first, size_t count) {
	std::set<std::string> ok_elements;
	if (first != nullptr && count != 0) {
		ok_elements.insert(first, first + count);
	}
	return [ok_elements](const char * guid) -> bool {
		return ok_elements.empty() || ok_elements.find(std::string(guid)) != ok_elements.end(); 
	};
}

void notify(const boost::format & fmt) {
	g_opts.notify_func(const_cast<char *>(fmt.str().c_str()));
}

void notify(char * msg) {
	g_opts.notify_func(msg);
}

void update_geometry_info(
	int * curr_points, 
	int * curr_edges,
	int * curr_faces,
	int * curr_solids,
	const solid & s)
{
	struct pntcmp : public std::binary_function<point, point, bool> {
		bool operator () (const point & a, const point & b) const {
			if (a.x == b.x) { return a.y == b.y ? a.z < b.z : a.y < b.y; }
			else { return a.x < b.x; }
		}
	};

	if (s.rep_type == REP_BREP) {
		const brep & b = s.rep.as_brep;
		std::set<point, pntcmp> pts;
		int halfedge_count = 0;
		for (size_t i = 0; i < b.face_count; ++i) {
			const face & f = b.faces[i];
			for (size_t j = 0; j < f.outer_boundary.vertex_count; ++j) {
				pts.insert(f.outer_boundary.vertices[j]);
			}
			halfedge_count += f.outer_boundary.vertex_count;
		}
		*curr_points = *curr_points + pts.size();
		*curr_edges = *curr_edges + halfedge_count / 2;
		*curr_faces = *curr_faces + b.face_count;
	}
	else {
		const extruded_area_solid & ext = s.rep.as_ext;
		int point_count = ext.area.outer_boundary.vertex_count * 2;
		int edge_count = ext.area.outer_boundary.vertex_count * 3;
		int face_count = ext.area.outer_boundary.vertex_count + 2;
		for (size_t i = 0; i < ext.area.void_count; ++i) {
			point_count += ext.area.voids[i].vertex_count * 2;
			edge_count += ext.area.voids[i].vertex_count * 3;
			face_count += ext.area.voids[i].vertex_count;
		}
		*curr_points = *curr_points + point_count;
		*curr_edges = *curr_edges + edge_count;
		*curr_faces = *curr_faces + face_count;
	}
	*curr_solids = *curr_solids + 1;
}

void gather_geometry_info(
	int * total_points,
	int * total_edges,
	int * total_faces,
	int * total_solids,
	size_t element_count,
	element_info ** elements,
	size_t space_count,
	space_info ** spaces)
{
	*total_points = *total_edges = *total_faces = *total_solids = 0;
	for (size_t i = 0; i < element_count; ++i) {
		update_geometry_info(
			total_points,
			total_edges,
			total_faces,
			total_solids,
			elements[i]->geometry);
	}	
	for (size_t i = 0; i < space_count; ++i) {
		update_geometry_info(
			total_points,
			total_edges,
			total_faces,
			total_solids,
			spaces[i]->geometry);
	}
}	

template <typename ApproximatedCurveRange>
boost::optional<float> calculate_corrected_area(
	const ApproximatedCurveRange & as,
	const polyloop & loop,
	double eps)
{
	// http://geomalgorithms.com/a01-_area.html
	point curr, next;
	double a = 0.0;
	double b = 0.0;
	double c = 0.0;
	double area_x = 0.0;
	double area_y = 0.0;
	double area_z = 0.0;
	double area;
	std::vector<ApproximatedCurveRange::const_iterator> forward_matches;
	std::vector<ApproximatedCurveRange::const_iterator> reverse_matches;
	for (size_t i = 0; i < loop.vertex_count; ++i) {
		curr = loop.vertices[i];
		next = loop.vertices[(i + 1) % loop.vertex_count];
		a += (curr.y - next.y) * (curr.z + next.z);
		b += (curr.z - next.z) * (curr.x + next.x);
		c += (curr.x - next.x) * (curr.y + next.y);
		area_x += curr.y * next.z - curr.z * next.y;
		area_y += curr.x * next.z - curr.z * next.x;
		area_z += curr.x * next.y - curr.y * next.x;
		for (auto apx = as.begin(); apx != as.end(); ++apx) {
			auto modify = apx->matches(curr.x, curr.y, curr.z,
									   next.x, next.y, next.z,
				                       eps);
			if (modify == approximated_curve::FORWARD_MATCH) {
				forward_matches.push_back(apx);
				break;
			}
			else if (modify == approximated_curve::REVERSE_MATCH) {
				reverse_matches.push_back(apx);
				break;
			}
		}
	}
	if (forward_matches.empty() && reverse_matches.empty()) {
		return boost::optional<float>();
	}
	direction_3 n(a, b, c);
	a = abs(a);
	b = abs(b);
	c = abs(c);
	double mag = sqrt(a * a + b * b + c * c);
	if (a >= b && a >= c) { area = abs(area_x / 2 * mag / a); }
	else if (b >= a && b >= c) { area = abs(area_y / 2 * mag / b); }
	else if (c >= a && c >= b) { area = abs(area_z / 2 * mag / c); }
	for (auto apx = as.begin(); apx != as.end(); ++apx) {
		if (number_collection<K>::are_effectively_perpendicular(
				n, 
				apx->original_plane_normal(), 
				eps))
		{
			return static_cast<float>(area * apx->true_length_ratio());
		}
	}
	for (auto m = forward_matches.begin(); m != forward_matches.end(); ++m) {
		if (share_sense(n, (*m)->original_plane_normal())) {
			area -= (*m)->true_area_on_left();
		}
		else { area += (*m)->true_area_on_left(); }
	}
	for (auto m = forward_matches.begin(); m != forward_matches.end(); ++m) {
		if (share_sense(n, (*m)->original_plane_normal())) {
			area += (*m)->true_area_on_left();
		}
		else { area -= (*m)->true_area_on_left(); }
	}
	return static_cast<float>(area);
}

float * calculate_corrected_areas(
	space_boundary ** sbs,
	size_t sb_count,
	const std::vector<approximated_curve> & as,
	double eps)
{
	std::map<space_boundary *, float> corrected_areas;
	for (size_t i = 0; i < sb_count; ++i) {
		auto corrected = calculate_corrected_area(as, sbs[i]->geometry, eps);
		assert(!corrected || *corrected >= 0.0);
		if (corrected) { corrected_areas[sbs[i]] = *corrected; }
	}
	float * res = (float *)malloc(sb_count * sizeof(float));
	for (size_t i = 0; i < sb_count; ++i) {
		auto c = corrected_areas.find(sbs[i]);
		if (c == corrected_areas.end() && sbs[i]->opposite) {
			c = corrected_areas.find(sbs[i]->opposite);
		}
		res[i] = c == corrected_areas.end() ? -1.0f : c->second;
	}
	return res;
}

} // namespace

ifcadapter_return_t execute(
	char * input_filename,
	char * output_filename, // NULL for no write-back
	sb_calculation_options opts,
	size_t * element_count,
	element_info *** elements,
	double ** composite_layer_dxs,
	double ** composite_layer_dys,
	double ** composite_layer_dzs,
	size_t * space_count,
	space_info *** spaces,
	size_t * sb_count,
	space_boundary *** sbs,
	float ** corrected_areas,
	int * total_points,
	int * total_edges,
	int * total_faces,
	int * total_solids)
{
	typedef boost::format fmt;
	g_opts = opts;
	*element_count = *space_count = *sb_count = 0;
	*elements = nullptr;
	*spaces = nullptr;
	*sbs = nullptr;
	*corrected_areas = nullptr;
	if (!input_filename) { return IFCADAPT_INVALID_ARGS; }
	number_collection<K> ctxt(EPS_MAGIC);
	number_collection<iK> output_ctxt(EPS_MAGIC);
	notify(fmt("Processing file %s.\n") % input_filename);

	ifc_interface::model m(input_filename);
	if (!m.loaded_ok()) {
		opts.error_func(const_cast<char *>((m.last_error() + "\n").c_str()));
		return IFCADAPT_IFC_ERROR;
	}

	std::vector<element_info *> shadings;
	std::vector<approximated_curve> approximated_curves;
	double lupm = m.length_units_per_meter();
	unit_scaler scaler(lupm);
	ifcadapter_return_t res = extract_from_model(
		&m,
		element_count,
		elements,
		composite_layer_dxs,
		composite_layer_dys,
		composite_layer_dzs,
		space_count,
		spaces,
		scaler,
		g_opts.notify_func,
		g_opts.warn_func,
		create_guid_filter(opts.element_filter, opts.element_filter_count),
		create_guid_filter(opts.space_filter, opts.space_filter_count),
		&ctxt,
		&shadings,
		&approximated_curves);
	if (res != IFCADAPT_OK) { return res; }
	else if (*space_count == 0) {
		opts.error_func("The model has no defined spaces.\n");
		return IFCADAPT_OK;
	}
	else if (*element_count == 0) {
		opts.error_func("The model has no defined building elements.\n");
		return IFCADAPT_OK;
	}
	gather_geometry_info(
		total_points,
		total_edges,
		total_faces,
		total_solids,
		*element_count,
		*elements,
		*space_count,
		*spaces);

	for (size_t i = 0; i < *element_count; ++i) {
		if ((*elements)[i]->id - 1 >= (int)*element_count) {
			opts.warn_func(
				"Internal error: an element has an invalid ID. IDF "
				"generation will likely fail.");
		}
	}
	
	// As far as the SBT core is concerned, everything is in meters.
	opts.length_units_per_meter = 1.0;
	sbt_return_t generate_res = 
		calculate_space_boundaries(
			*element_count, 
			*elements, 
			*space_count, 
			*spaces, 
			sb_count, 
			sbs, 
			opts);
	if (generate_res == SBT_OK) {

		element_id_t max_id = -1;
		for (size_t i = 0; i < *sb_count; ++i) {
			for (size_t j = 0; j < (*sbs)[i]->material_layer_count; ++j) {
				if ((*sbs)[i]->layers[j] > max_id) {
					max_id = (*sbs)[i]->layers[j];
				}
			}
		}
		if (max_id - 1 >= (int)*element_count) {
			opts.warn_func(
				"Internal error: A space boundary material layer "
				"references an invalid element. IDF generation will "
				"likely fail.\n");
		}

		if (output_filename != nullptr) {
			m.invalidate_all_ownership_pointers();
			m.remove_all_space_boundaries();
			// add_to_model figures out the right spaces by re-extracting 
			// them based on guids.
			res = add_to_model(
					&m, 
					*sb_count, 
					*sbs, 
					scaler,
					opts.notify_func, 
					&output_ctxt);
			if (res == IFCADAPT_OK)
			{
				notify(fmt("Writing model to %s...") % output_filename);
				m.write(output_filename);
				notify("done.\n");
			}
		}

		auto total_e_count = *element_count + shadings.size();
		auto total_e_size = sizeof(element_info *) * total_e_count;
		auto total_cdir_size = sizeof(double) * total_e_count;
		*elements = (element_info **)realloc(*elements, total_e_size);
		*composite_layer_dxs = 
			(double *)realloc(*composite_layer_dxs, total_cdir_size);
		*composite_layer_dys = 
			(double *)realloc(*composite_layer_dys, total_cdir_size);
		*composite_layer_dzs = 
			(double *)realloc(*composite_layer_dzs, total_cdir_size);
		for (size_t i = 0; i < shadings.size(); ++i) {
			(*elements)[*element_count + i] = shadings[i];
			(*composite_layer_dxs)[*element_count + i] = 0;
			(*composite_layer_dys)[*element_count + i] = 0;
			(*composite_layer_dzs)[*element_count + i] = 0;
		}
		*element_count = *element_count + shadings.size();

		*corrected_areas = calculate_corrected_areas(
			*sbs,
			*sb_count,
			approximated_curves,
			EPS_MAGIC);

		return IFCADAPT_OK;
	}
	else if (generate_res == SBT_STACK_OVERFLOW) {
		return IFCADAPT_STACK_OVERFLOW;
	}
	else {
		return IFCADAPT_UNKNOWN;
	}
}

void release_elements(element_info ** elements, size_t count) {
	release_list(elements, count);
}

void release_spaces(space_info ** spaces, size_t count) {
	release_list(spaces, count);
}

void release_corrected_areas(float * corrected_areas) {
	free(corrected_areas);
}