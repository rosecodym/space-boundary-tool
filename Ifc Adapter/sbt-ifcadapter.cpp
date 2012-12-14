#include "precompiled.h"

#include "edm_wrapper.h"
#include "get_length_units_per_meter.h"
#include "model_operations.h"
#include "number_collection.h"
#include "reassign_bounded_spaces.h"
#include "release.h"
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

void scale_shadings(
	std::vector<element_info *> * shadings,
	const unit_scaler & scaler)
{
	for (auto p = shadings->begin(); p != shadings->end(); ++p) {
		scale_solid(&(*p)->geometry, scaler);
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

void notify(const boost::format & fmt) {
	g_opts.notify_func(const_cast<char *>(fmt.str().c_str()));
}

void notify(char * msg) {
	g_opts.notify_func(msg);
}

} // namespace

ifcadapter_return_t execute(
	const char * input_filename,
	const char * output_filename, // NULL for no write-back
	sb_calculation_options opts,
	size_t * element_count,
	element_info *** elements,
	double ** composite_layer_dxs,
	double ** composite_layer_dys,
	double ** composite_layer_dzs,
	size_t * space_count,
	space_info *** spaces,
	size_t * sb_count,
	space_boundary *** sbs)
{
	typedef boost::format fmt;
	try {
		g_opts = opts;
		*element_count = *space_count = *sb_count = 0;
		*elements = nullptr;
		*spaces = nullptr;
		*sbs = nullptr;
		if (!input_filename) { return IFCADAPT_INVALID_ARGS; }
		number_collection<K> ctxt(EPS_MAGIC / 20); // magic divided by magic
		notify(fmt("Processing file %s.\n") % input_filename);
		edm_wrapper edm;
		cppw::Open_model model = edm.load_ifc_file(input_filename);
		std::vector<element_info *> shadings;
		// The length-unit information is stored redundantly because it's due
		// for a refactoring. This is why globals are bad, folks (I don't want
		// to just take stuff out because I don't know what's touching g_opts).
		opts.length_units_per_meter = get_length_units_per_meter(model);
		unit_scaler scaler(model);
		ifcadapter_return_t res = extract_from_model(
			model,
			element_count,
			elements,
			space_count,
			spaces,
			g_opts.notify_func,
			create_guid_filter(opts.element_filter, opts.element_filter_count),
			create_guid_filter(opts.space_filter, opts.space_filter_count),
			&ctxt,
			&shadings);
		if (res != IFCADAPT_OK) { return res; }
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
			if (output_filename != nullptr) {
				clear_sbs(&model);
				// add_to_model figures out the right spaces by re-extracting 
				// them based on guids.
				res = add_to_model(
						model, 
						*sb_count, 
						*sbs, 
						opts.notify_func, 
						&ctxt);
				if (res == IFCADAPT_OK)
				{
					notify(fmt("Writing model to %s...") % output_filename);
					edm.write_ifc_file(output_filename);
					notify("done.\n");
				}
			}
			scale_elements(*elements, *element_count, scaler);
			scale_spaces(*spaces, *space_count, scaler);
			scale_space_boundaries(*sbs, *sb_count, scaler);
			scale_shadings(&shadings, scaler);

			auto total_e_count = *element_count + shadings.size();
			auto total_e_size = sizeof(element_info *) * total_e_count;
			*elements = (element_info **)realloc(*elements, total_e_size);
			for (size_t i = 0; i < shadings.size(); ++i) {
				(*elements)[*element_count + i] = shadings[i];
			}
			*element_count = *element_count + shadings.size();

			return IFCADAPT_OK;
		}
		else if (generate_res == SBT_TOO_COMPLICATED) {
			return IFCADAPT_TOO_COMPLICATED;
		}
		else {
			return IFCADAPT_UNKNOWN;
		}
	}
	catch (cppw::Error & e) {
		notify(fmt("edm error: %s\n") % e.message.data());
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

void release_elements(element_info ** elements, size_t count) {
	release_list(elements, count);
}

void release_spaces(space_info ** spaces, size_t count) {
	release_list(spaces, count);
}

void free_space_boundaries(space_boundary ** sbs, size_t count) {
	release_space_boundaries(sbs, count);
}