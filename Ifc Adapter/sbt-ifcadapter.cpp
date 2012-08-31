#include "precompiled.h"

#include "edm_wrapper.h"
#include "get_length_units_per_meter.h"
#include "model_operations.h"
#include "number_collection.h"
#include "reassign_bounded_spaces.h"
#include "unit_scaler.h"

#include "sbt-ifcadapter.h"

bool found_freestanding = false;

sb_calculation_options g_opts;

namespace {

void clear_sbs(cppw::Open_model * model) {
	cppw::Application_aggregate sbs = model->get_set_of("IfcRelSpaceBoundary", cppw::include_subtypes);
	for (int i = 0; i < sbs.count(); ++i) {
		((cppw::Application_instance)sbs.get_()).remove();
	}
}

void generate_sb_summary(sb_counts * counts, space_boundary ** sbs, size_t sb_count) {
	if (counts == NULL) {
		return;
	}

	std::map<std::string, size_t> spaces;
	for (size_t i = 0; i < sb_count; ++i) {
		if (!sbs[i]->lies_on_outside) {
			spaces[sbs[i]->bounded_space->id] = 0;
		}
	}

	counts->bounded_space_count = spaces.size() + 1;
	counts->space_guids = (char (*)[SPACE_ID_MAX_LEN])malloc(sizeof(char[SPACE_ID_MAX_LEN]) * counts->bounded_space_count);

	strcpy(counts->space_guids[0], "Total");

	size_t i = 1; 
	std::for_each(spaces.begin(), spaces.end(), [&i, &counts](std::pair<const std::string, size_t> & sp) {
		strcpy(counts->space_guids[i], sp.first.c_str());
		sp.second = i;
		++i;
	});

	size_t counts_arr_size = sizeof(int) * counts->bounded_space_count;
	counts->level_2_physical_external = (int *)malloc(counts_arr_size);
	memset(counts->level_2_physical_external, 0, counts_arr_size);
	counts->level_2_physical_internal = (int *)malloc(counts_arr_size);
	memset(counts->level_2_physical_internal, 0, counts_arr_size);
	counts->level_3_external = (int *)malloc(counts_arr_size);
	memset(counts->level_3_external, 0, counts_arr_size);
	counts->level_3_internal = (int *)malloc(counts_arr_size);
	memset(counts->level_3_internal, 0, counts_arr_size);
	counts->level_4 = (int *)malloc(counts_arr_size);
	memset(counts->level_4, 0, counts_arr_size);
	counts->level_5 = (int *)malloc(counts_arr_size);
	memset(counts->level_5, 0, counts_arr_size);
	counts->virt = (int *)malloc(counts_arr_size);
	memset(counts->virt, 0, counts_arr_size);

	for (size_t i = 0; i < sb_count; ++i) {
		if (!sbs[i]->lies_on_outside) {
			size_t sp_ix = spaces[sbs[i]->bounded_space->id];
			if (sbs[i]->is_virtual) {
				++counts->virt[0];
				++counts->virt[sp_ix];
			}
			else if (sbs[i]->level == 2) {
				if (!sbs[i]->opposite) {
					++counts->level_2_physical_external[0];
					++counts->level_2_physical_external[sp_ix];
				}
				else {
					++counts->level_2_physical_internal[0];
					++counts->level_2_physical_internal[sp_ix];
				}
			}
			else if (sbs[i]->level == 3) {
				++counts->level_3_internal[0];
				++counts->level_3_internal[sp_ix];
			}
			else if (sbs[i]->level == 4) {
				++counts->level_4[0];
				++counts->level_4[sp_ix];
			}
			else if (sbs[i]->level == 5) {
				++counts->level_5[0];
				++counts->level_5[sp_ix];
			}
		}
	}
}

void scale_point(point * p, const unit_scaler & scaler) {
	p->x = scaler.length_in(p->x);
	p->y = scaler.length_in(p->y);
	p->z = scaler.length_in(p->z);
}

void scale_loop(polyloop * loop, const unit_scaler & scaler) {
	for (size_t i = 0; i < loop->vertex_count; ++i) {
		scale_point(&loop->vertices[i], scaler);
	}
}

void scale_face(face * f, const unit_scaler & scaler) {
	scale_loop(&f->outer_boundary, scaler);
	for (size_t i = 0; i < f->void_count; ++i) {
		scale_loop(&f->voids[i], scaler);
	}
}

void scale_brep(brep * b, const unit_scaler & scaler) {
	for (size_t i = 0; i < b->face_count; ++i) {
		scale_face(&b->faces[i], scaler);
	}
}

void scale_ext(extruded_area_solid * e, const unit_scaler & scaler) {
	e->extrusion_depth = scaler.length_in(e->extrusion_depth);
	scale_face(&e->area, scaler);
}

void scale_solid(solid * s, const unit_scaler & scaler) {
	if (s->rep_type == REP_BREP) { scale_brep(&s->rep.as_brep, scaler); }
	else { scale_ext(&s->rep.as_ext, scaler); }
}

void scale_elements(element_info ** elements, size_t count, const unit_scaler & scaler) {
	for (size_t i = 0; i < count; ++i) {
		scale_solid(&elements[i]->geometry, scaler);
	}
}

void scale_spaces(space_info ** spaces, size_t count, const unit_scaler & scaler) {
	for (size_t i = 0; i < count; ++i) {
		scale_solid(&spaces[i]->geometry, scaler);
	}
}

void scale_space_boundaries(space_boundary ** sbs, size_t count, const unit_scaler & scaler) {
	for (size_t i = 0; i < count; ++i) {
		scale_loop(&sbs[i]->geometry, scaler);
		for (size_t j = 0; j < sbs[i]->material_layer_count; ++j) {
			sbs[i]->thicknesses[j] = scaler.length_in(sbs[i]->thicknesses[j]);
		}
	}
}

std::function<bool(const char *)> create_guid_filter(char ** first, size_t count) {
	std::set<std::string> ok_elements;
	if (first != nullptr && count != 0) {
		ok_elements.insert(first, first + count);
	}
	return [ok_elements](const char * guid) -> bool {
		return ok_elements.empty() || ok_elements.find(std::string(guid)) != ok_elements.end(); 
	};
}

} // namespace

ifcadapter_return_t add_to_ifc_file(const char * input_filename, const char * output_filename, sb_calculation_options options, sb_counts * counts) {
	char buf[256];
	try {
		g_opts = options;
		number_collection ctxt(g_opts.equality_tolerance / 20);
		sprintf(buf, "Processing file %s", input_filename);
		options.notify_func(buf);
		edm_wrapper edm;
		cppw::Open_model model = edm.load_ifc_file(input_filename);
		options.notify_func("File loaded.\n");
		element_info ** elements;
		space_boundary ** sbs;
		space_info ** loaded_spaces;
		size_t element_count;
		size_t sb_count;
		size_t loaded_space_count;
		ifcadapter_return_t res = extract_from_model(
			model, 
			&element_count, 
			&elements, 
			&loaded_space_count, 
			&loaded_spaces, 
			options.notify_func, 
			unit_scaler::identity_scaler, 
			create_guid_filter(options.element_filter, options.element_filter_count),
			&ctxt);
		double length_units_per_meter = get_length_units_per_meter(model);
		if (res == IFCADAPT_OK) {
			sb_calculation_options opts;
			opts = options;
			opts.max_pair_distance *= length_units_per_meter;
			sbt_return_t generate_res = calculate_space_boundaries(element_count, elements, loaded_space_count, loaded_spaces, &sb_count, &sbs, opts);
			if (generate_res == SBT_OK) {
				generate_sb_summary(counts, sbs, sb_count);
				sprintf(buf, "Generated count summary.\n");
				options.notify_func(buf);
				clear_sbs(&model);
				// add_to_model figures out the right spaces by re-extracting them based on guids
				if (add_to_model(model, sb_count, sbs, options.notify_func, unit_scaler::identity_scaler, &ctxt) == IFCADAPT_OK) {
					sprintf(buf, "Writing model to %s...", output_filename);
					options.notify_func(buf);
					edm.write_ifc_file(output_filename);
					options.notify_func("done.\n");
					res = IFCADAPT_OK;
				}
				free_sb_list(sbs, sb_count);
			}
			else if (generate_res == SBT_TOO_COMPLICATED) {
				res = IFCADAPT_TOO_COMPLICATED;
			}
			else {
				res = IFCADAPT_UNKNOWN;
			}
			free_element_list(elements, element_count);
			free_space_list(loaded_spaces, loaded_space_count);
		}
		return res;
	}
	catch (cppw::Error & e) {
		sprintf(buf, "edm error: %s\n", e.message.data());
		options.error_func(buf);
		return IFCADAPT_EDM_ERROR;
	}
}

ifcadapter_return_t load_and_run_from(
	const char * input_filename,
	const char * output_filename, // NULL if you don't want to write back
	sb_calculation_options options,
	element_info *** elements,
	size_t * element_count,
	space_info *** spaces,
	size_t * space_count,
	space_boundary *** sbs,
	size_t * total_sb_count)
{
	char buf[256];
	try {
		g_opts = options;
		number_collection ctxt(g_opts.equality_tolerance / 20);
		sprintf(buf, "Processing file %s", input_filename);
		options.notify_func(buf);
		edm_wrapper edm;
		cppw::Open_model model = edm.load_ifc_file(input_filename);
		options.notify_func("File loaded.\n");
		unit_scaler scaler(model);
		ifcadapter_return_t res = extract_from_model(
			model, 
			element_count, 
			elements, 
			space_count, 
			spaces, 
			options.notify_func, 
			unit_scaler::identity_scaler, 
			create_guid_filter(options.element_filter, options.element_filter_count),
			&ctxt);
		double length_units_per_meter = get_length_units_per_meter(model);
		if (res == IFCADAPT_OK) {
			sb_calculation_options opts;
			opts = options;
			opts.max_pair_distance *= length_units_per_meter;
			sbt_return_t generate_res = calculate_space_boundaries(*element_count, *elements, *space_count, *spaces, total_sb_count, sbs, opts);
			if (generate_res == SBT_OK && output_filename != nullptr) {
				// add_to_model figures out the right spaces by re-extracting them based on guids
				clear_sbs(&model);
				options.notify_func("Existing space boundaries removed from model.\n");
				if (add_to_model(model, *total_sb_count, *sbs, options.notify_func, /*scaler*/unit_scaler::identity_scaler, &ctxt) == IFCADAPT_OK) {
					sprintf(buf, "Writing model to %s...", output_filename);
					options.notify_func(buf);
					edm.write_ifc_file(output_filename);
					options.notify_func("done.\n");
					scale_elements(*elements, *element_count, scaler);
					scale_spaces(*spaces, *space_count, scaler);
					scale_space_boundaries(*sbs, *total_sb_count, scaler);
					res = IFCADAPT_OK;
				}
			}
			else if (generate_res == SBT_TOO_COMPLICATED) {
				res = IFCADAPT_TOO_COMPLICATED;
			}
			else {
				res = generate_res == SBT_OK ? IFCADAPT_OK : IFCADAPT_UNKNOWN;
			}
		}
		return res;
	}
	catch (cppw::Error & e) {
		sprintf(buf, "edm error: %s\n", e.message.data());
		options.error_func(buf);
		return IFCADAPT_EDM_ERROR;
	}
}

void free_sb_counts(sb_counts counts) {
	free(counts.space_guids);
	free(counts.level_2_physical_external);
	free(counts.level_2_physical_internal);
	free(counts.level_3_external);
	free(counts.level_3_internal);
	free(counts.level_4);
	free(counts.level_5);
	free(counts.virt);
}

void free_elements(element_info ** elements, size_t count) {
	free_element_list(elements, count);
}

void free_spaces(space_info ** spaces, size_t count) {
	free_space_list(spaces, count);
}

void free_space_boundaries(space_boundary ** sbs, size_t count) {
	free_sb_list(sbs, count);
}