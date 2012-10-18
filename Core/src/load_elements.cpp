#include "precompiled.h"

#include "element.h"
#include "exceptions.h"
#include "guid_filter.h"
#include "report.h"

#include "load_elements.h"

using namespace reporting;

std::vector<element> load_elements(element_info ** infos, size_t count, equality_context * c, const guid_filter & filter) {
	report_progress(boost::format("Beginning loading for %u elements.\n") % count);

	std::vector<element> complex_elements;
	for (size_t i = 0; i < count; ++i) {
		try {
			if (filter(infos[i]->id)) {
				complex_elements.push_back(element(infos[i], c));
			}
		}
		catch (unsupported_geometry_exception & ex) {
			report_warning(boost::format("Element %s has unsupported geometry (%s). It will be ignored.\n") % infos[i]->id % ex.condition());
		}
		catch (unknown_geometry_rep_exception & /*ex*/) {
			report_warning(boost::format("Element %s has unknown internal geometry representation type. It will be ignored. Please report this SBT bug.\n") % infos[i]->id);
		}
	}

	typedef std::vector<element>::iterator element_iterator;
	typedef CGAL::Box_intersection_d::Box_with_handle_d<double, 3, element_iterator> element_box;

	std::vector<element_box> walls;
	std::vector<element_box> slabs;
	std::vector<element_box> columns;

	for (auto e = complex_elements.begin(); e != complex_elements.end(); ++e) {
		element_box box(e->bounding_box(), e);
		if (e->type() == WALL) { walls.push_back(box); }
		else if (e->type() == SLAB) { slabs.push_back(box); }
		else if (e->type() == COLUMN) { columns.push_back(box); }
	}

	report_progress(boost::format("Got bounding boxes (%u walls, %u slabs, %u columns).\n") % walls.size() % slabs.size() % columns.size());

	bool need_wall_column_check = !walls.empty() && !columns.empty();
	bool need_slab_column_check = !slabs.empty() && !columns.empty();

	if (need_wall_column_check) {
		for (auto w = walls.begin(); w != walls.end(); ++w) {

			report_progress(boost::format("Resolving wall %s") % w->handle()->source_id().c_str());
			bool performed_any_subtractions = false;

			if (need_wall_column_check) {
				boost::for_each(columns, [w, &performed_any_subtractions, c](const element_box & col) {
					if (CGAL::do_overlap(w->bbox(), col.bbox())) {
						w->handle()->subtract_geometry_of(*col.handle(), c);
						report_progress(".");
						performed_any_subtractions = true;
					}
				});
			}

			report_progress(performed_any_subtractions ? "done.\n" : " - no resolution necessary.\n");
		}
	}

	if (need_slab_column_check) {
		for (auto col = columns.begin(); col != columns.end(); ++col) {

			report_progress(boost::format("Resolving column %s") % col->handle()->source_id().c_str());
			bool performed_any_subtractions = false;

			boost::for_each(slabs, [col, &performed_any_subtractions, c](const element_box & s) {
				if (CGAL::do_overlap(col->bbox(), s.handle()->bounding_box())) {
					col->handle()->subtract_geometry_of(*s.handle(), c);
					report_progress(".");
					performed_any_subtractions = true;
				}
			});

			report_progress(performed_any_subtractions ? "done.\n" : " - no resolution necessary.\n");
		}
	}

	std::vector<element> res;
	for (auto e = complex_elements.begin(); e != complex_elements.end(); ++e) {
		element::explode_to_single_volumes(std::move(*e), c, std::back_inserter(res));
	}

	return res;
}